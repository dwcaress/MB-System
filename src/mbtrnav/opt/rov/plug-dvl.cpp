#include "trnx_plugin.hpp"

// g++ -c -g -O2 -std=c++11 -I. -I../../utils -I../../qnx-utils -I../../terrain-nav -I../../newmat -I../../trnw -I/usr/local/include -I/opt/local/include -fPIC plug-dvl.cpp

// g++ -shared -o libdvl.so plug-dvl.o -L../../bin -L/usr/local/lib -L/opt/local/lib -L. -lnewmat -lgeolib -ltrnw -lqnx -lmb1 -ltrn -ludpms -ltrncli -lnetcdf -llcm


//extern "C" void transform_dvl(trn::bath_info *bi, trn::att_info *ai, dvlgeo *geo, mb1_t *r_snd);
//extern "C" int cb_proto_dvl(void *pargs);


// It probably doesn't make sense to filter DVL beams
// using mbtrnpp, since it assumes they are distributed
// in a linear array.
void transform_dvl(trn::bath_info *bi, trn::att_info *ai, dvlgeo *geo, mb1_t *r_snd)
{
    int FN_DEBUG_HI = 6;
    int FN_DEBUG = 5;

    if(NULL == geo || geo->beam_count<=0){
        fprintf(stderr, "%s - geometry error : beams<=0\n", __func__);
        return;
    }
    if(NULL == r_snd || NULL == ai|| NULL == bi){
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
        return;
    }

    // vehicle attitude (relative to NED)
    // r/p/y (phi/theta/psi)
    // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
    double VATT[3] = {ai->roll(), ai->pitch(), 0.};

    // sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // wrt sensor mounted across track, b[0] port, downward facing
    double SROT[3] = { DTR(geo->svr_deg[0]), DTR(geo->svr_deg[1]), DTR(geo->svr_deg[2])};

    // sensor mounting translation offsets (relative to vehicle CRP, meters)
    // +x: fwd +y: stbd, +z:down (aka FSK, fwd,stbd,keel)
    // TODO T is for transform use rSV
    double STRN[3] = {geo->svt_m[0], geo->svt_m[1], geo->svt_m[2]};

    // beam components in sensor frame
    Matrix beams_SF = trnx_utils::dvl_sframe_components(bi, geo);

    TRN_NDPRINT(FN_DEBUG, "%s: --- \n",__func__);

    TRN_NDPRINT(FN_DEBUG, "VATT[%.3lf, %.3lf, %.3lf]\n", VATT[0], VATT[1], VATT[2]);
    TRN_NDPRINT(FN_DEBUG, "SROT[%.3lf, %.3lf, %.3lf]\n", SROT[0], SROT[1], SROT[2]);
    TRN_NDPRINT(FN_DEBUG, "STRN[%.3lf, %.3lf, %.3lf]\n", STRN[0], STRN[1], STRN[2]);

    const char *pinv = (ai->flags().is_set(trn::AF_INVERT_PITCH)? "(p-)" :"(p+)");

    TRN_NDPRINT(FN_DEBUG, "VATT (deg) [%.2lf, %.2lf, %.2lf (%.2lf)] %s\n",
                Math::radToDeg(VATT[0]), Math::radToDeg(VATT[1]), Math::radToDeg(VATT[2]), Math::radToDeg(ai->heading()), pinv);
    TRN_NDPRINT(FN_DEBUG,"\n");

    // generate coordinate tranformation matrices
    // mounting rotation matrix
    Matrix mat_SROT = trnx_utils::affine321Rotation(SROT);
    // mounting translation matrix
    Matrix mat_STRN = trnx_utils::affineTranslation(STRN);
    // vehicle attitude (pitch, roll, heading)
    Matrix mat_VATT = trnx_utils::affine321Rotation(VATT);

    // combine to get composite tranformation
    // order is significant:

    // apply sensor rotation, translation
    Matrix G = mat_SROT * mat_STRN;
    // apply vehicle attitude
    Matrix Q = mat_VATT * G;

    // apply coordinate transforms
    Matrix beams_VF = Q * beams_SF;

    if(trn_debug::get()->debug() >= FN_DEBUG_HI){

        TRN_NDPRINT(5, "\n");
        trnx_utils::matrix_show(mat_SROT, "mat_SROT");
        TRN_NDPRINT(5, "\n");
        trnx_utils::matrix_show(mat_STRN, "mat_STRN");
        TRN_NDPRINT(5, "\n");
        trnx_utils::matrix_show(mat_VATT, "mat_VATT");
        TRN_NDPRINT(5, "\n");
        trnx_utils::matrix_show(G, "G");
        TRN_NDPRINT(5, "\n");
        trnx_utils::matrix_show(Q, "Q");
        TRN_NDPRINT(5, "\n");
    }

    // fill in the MB1 record using transformed beams
    std::list<trn::beam_tup> beams = bi->beams_raw();
    std::list<trn::beam_tup>::iterator it;

    // zero- and one-based indexs
    int idx[2] = {0, 1};
    for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
    {
        // write beam data to MB1 sounding
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);

        // beam number (0-indexed)
        int b = std::get<0>(bt);
        double range = std::get<1>(bt);
        // beam components WF x,y,z
        // matrix row/col (1 indexed)
        r_snd->beams[idx[0]].beam_num = b;
        r_snd->beams[idx[0]].rhox = range * beams_VF(1, idx[1]);
        r_snd->beams[idx[0]].rhoy = range * beams_VF(2, idx[1]);
        r_snd->beams[idx[0]].rhoz = range * beams_VF(3, idx[1]);

        if(trn_debug::get()->debug() >= 5){

            // calculated beam range (should match measured range)
            double rho[3] = {r_snd->beams[idx[0]].rhox, r_snd->beams[idx[0]].rhoy, r_snd->beams[idx[0]].rhoz};

            double rhoNorm = trnx_utils::vnorm( rho );

            // calculate component angles wrt vehicle axes
            double axr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhox/range));
            double ayr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhoy/range));
            double azr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhoz/range));

            TRN_NDPRINT(FN_DEBUG_HI, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf]     ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
                        __func__, b, range, rhoNorm,
                        r_snd->beams[idx[0]].rhox,
                        r_snd->beams[idx[0]].rhoy,
                        r_snd->beams[idx[0]].rhoz,
                        Math::radToDeg(axr),
                        Math::radToDeg(ayr),
                        Math::radToDeg(azr)
                        );
        }
    }
    TRN_NDPRINT(FN_DEBUG, "%s: --- \n\n",__func__);

    return;
}

// input: DVL
// publish to: TRN server
// expects:
// bi     : bathymetry, DVL or deltaT (on vehicle frame)
// ni     : navigation (on vehicle frame)
// ai     : attitude (on vehicle frame)
// vi     : velocity (optional, may be NULL)
int cb_proto_dvl(void *pargs)
{
    int retval=-1;
    static uint32_t ping_number = 0;
    int FN_DEBUG = 5;

    TRN_NDPRINT(3, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

    trn::trnxpp::callback_res_t *cb_res = static_cast<trn::trnxpp::callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;
    trnxpp_cfg *cfg = cb_res->cfg;

    cfg->stats().trn_cb_n++;

    // iterate over contexts
    std::vector<trn::trnxpp_ctx *>::iterator it;
    for(it = xpp->ctx_list_begin(); it != xpp->ctx_list_end(); it++)
    {
        trn::trnxpp_ctx *ctx = (*it);
        // if context defined for this callback
        if(ctx == nullptr || !ctx->has_callback("cb_proto_dvl"))
        {
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(FN_DEBUG, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        int err_count = 0;

        std::string *bkey[1] = {ctx->bath_input_chan(0)};
        std::string *nkey = ctx->nav_input_chan(0);
        std::string *akey[1] = {ctx->att_input_chan(0)};
        std::string *vkey = ctx->vel_input_chan(0);

        if(bkey[0] == nullptr || nkey == nullptr || akey[0] == nullptr || vkey == nullptr)
        {
            TRN_NDPRINT(FN_DEBUG, "%s:%d WARN - NULL input key\n", __func__, __LINE__);
            err_count++;
            continue;
        }

        trn::bath_info *bi = xpp->get_bath_info(*bkey[0]);
        trn::nav_info *ni = xpp->get_nav_info(*nkey);
        trn::att_info *ai = xpp->get_att_info(*akey[0]);
        trn::vel_info *vi = xpp->get_vel_info(*vkey);

        if(bi == nullptr || ni == nullptr || ai == nullptr || vi == nullptr)
        {
            fprintf(stderr,"%s:%d WARN - NULL info instance\n", __func__, __LINE__);
            err_count++;
        }


        // if no errors, bs/bp pointers have been validated

        double nav_time = ni->time_usec()/1e6;

        mb1_t *snd = trnx_utils::lcm_to_mb1(bi, ni, ai);

        beam_geometry *bgeo = xpp->lookup_geo(*bkey[0], trn::BT_DVL);

        dvlgeo *geo = static_cast<dvlgeo *>(bgeo);
        trn::bath_input *bp[1] = {xpp->get_bath_input(*bkey[0])};
        int trn_type = bp[0]->bath_input_type();

        // compute beam components in vehicle frame
        transform_dvl(bi, ai, geo, snd);

        // check modulus
        if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0){


            // construct poseT/measT TRN inputs

            poseT *pt = trnx_utils::mb1_to_pose(snd, ai, (long)ctx->utm_zone());
            measT *mt = trnx_utils::mb1_to_meas(snd, ai, trn_type, (long)ctx->utm_zone());

            // publish update TRN, publish estimate to TRN, LCM
            ctx->pub_trn(nav_time, pt, mt, trn_type, xpp->pub_list(), cfg);

            if(pt != nullptr)
                delete pt;
            if(mt != nullptr)
                delete mt;

        } else {
            TRN_NDPRINT(FN_DEBUG, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
        }
        ctx->inc_cbcount();


        // write to CSV
        if(ctx->write_mb1_csv(snd, bi, ai, vi) > 0){
            cfg->stats().trn_csv_n++;
        }
        if(snd != nullptr)
            mb1_destroy(&snd);

        if(bi != nullptr)
            delete bi;
        if(ai != nullptr)
            delete ai;
        if(ni != nullptr)
            delete ni;
        if(vi != nullptr)
            delete vi;
    }

    ping_number++;

    return retval;
}
