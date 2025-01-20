#include "trnx_plugin.hpp"

// g++ -c -g -O2 -std=c++11 -I. -I../../utils -I../../qnx-utils -I../../terrain-nav -I../../newmat -I../../trnw -I/usr/local/include -I/opt/local/include -fPIC plug-idt.cpp

// g++ -shared -o libdvl.so plug-idt.o -L../../bin -L/usr/local/lib -L/opt/local/lib -L. -lnewmat -lgeolib -ltrnw -lqnx -lmb1 -ltrn -ludpms -ltrncli -lnetcdf -llcm -ldvl

void transform_deltat(trn::bath_info *bi, trn::att_info *ai, mbgeo *geo, mb1_t *r_snd)
{
    if(NULL == geo || geo->beam_count<=0){
        fprintf(stderr, "%s - geometry error : beams<=0\n", __func__);
        return;
    }

    if(NULL == r_snd || NULL == ai|| NULL == bi){
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
        return;
    }

    // vehicle attitude (relative to NED, radians)
    // r/p/y  (phi/theta/psi)
    // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
    double VATT[3] = {ai->roll(), ai->pitch(), 0.};

    // sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // wrt sensor mounted across track, b[0] port, downward facing
    double SROT[3] = { DTR(geo->svr_deg[0]), DTR(geo->svr_deg[1]), DTR(geo->svr_deg[2])};

    // sensor mounting translation offsets (relative to vehicle CRP, meters)
    // +x: fwd +y: stbd, +z:down
    double STRN[3] = {geo->svt_m[0], geo->svt_m[1], geo->svt_m[2]};

    Matrix beams_SF = trnx_utils::mb_sframe_components(bi, geo);

    TRN_NDPRINT(TRNDL_PLUGIDT, "%s: --- \n",__func__);

    TRN_NDPRINT(TRNDL_PLUGIDT, "VATT[%.3lf, %.3lf, %.3lf]\n", VATT[0], VATT[1], VATT[2]);
    TRN_NDPRINT(TRNDL_PLUGIDT, "SROT[%.3lf, %.3lf, %.3lf]\n", SROT[0], SROT[1], SROT[2]);
    TRN_NDPRINT(TRNDL_PLUGIDT, "STRN[%.3lf, %.3lf, %.3lf]\n", STRN[0], STRN[1], STRN[2]);

    const char *pinv = (ai->flags().is_set(trn::AF_INVERT_PITCH)? "(p-)" :"(p+)");

    TRN_NDPRINT(TRNDL_PLUGIDT, "VATT (deg) [%.2lf, %.2lf, %.2lf (%.2lf)] %s\n",
                Math::radToDeg(VATT[0]), Math::radToDeg(VATT[1]), Math::radToDeg(VATT[2]), Math::radToDeg(ai->heading()), pinv);
    TRN_NDPRINT(5,"\n");

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

    if(trn_debug::get()->debug() >= TRNDL_PLUGIDT_H){

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

            TRN_NDPRINT(TRNDL_PLUGIDT_H, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf]     ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
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
    TRN_NDPRINT(TRNDL_PLUGIDT, "%s: --- \n\n",__func__);

    return;
}
// input: DeltaT or DVL
// publish to: mbtrnpp, TRN server
// expects:
// bi     : bathymetry, DVL or deltaT (on vehicle frame)
// ni     : navigation (on vehicle frame)
// ai     : attitude (on vehicle frame)
// vi     : velocity (optional, may be NULL)
int cb_proto_deltat(void *pargs)
{
    int retval=-1;

    TRN_NDPRINT(TRNDL_PLUGIDT_H, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

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
        if(ctx == nullptr || !ctx->has_callback("cb_proto_deltat"))
        {
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(TRNDL_PLUGIDT, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        std::string *bkey = ctx->bath_input_chan(0);
        std::string *nkey = ctx->nav_input_chan(0);
        std::string *akey = ctx->att_input_chan(0);
        std::string *vkey = ctx->vel_input_chan(0);

        // vi is optional
        if(bkey == nullptr || nkey == nullptr || akey == nullptr)
        {
            TRN_NDPRINT(TRNDL_PLUGIDT, "%s:%d WARN - NULL input key\n", __func__, __LINE__);
            continue;
        }

        trn::bath_info *bi = xpp->get_bath_info(*bkey);
        trn::nav_info *ni = xpp->get_nav_info(*nkey);
        trn::att_info *ai = xpp->get_att_info(*akey);
        trn::vel_info *vi = (vkey == nullptr ? nullptr : xpp->get_vel_info(*vkey));

        if(bi == nullptr || ni == nullptr || ai == nullptr || vi == nullptr)
        {
            TRN_NDPRINT(TRNDL_PLUGIDT, "%s:%d WARN - NULL info instance\n", __func__, __LINE__);
            TRN_NDPRINT(TRNDL_PLUGIDT, "%s:%d   bi[%p] ni[%p] ai[%p] vi[%p]\n", __func__, __LINE__);
        }

        std::string *bath_0_key = ctx->bath_input_chan(0);
        TRN_NDPRINT(TRNDL_PLUGIDT_H, "BATHINST.%s : %s\n",bath_0_key->c_str(), bi->bathstr());

        size_t n_beams = bi->beam_count();

        if(n_beams>0){

            // generate MB1 sounding (raw beams)
            mb1_t *snd = trnx_utils::lcm_to_mb1(bi, ni, ai);

            // if streams_ok, bs/bp pointers have been validated
            trn::bath_input *bp = xpp->get_bath_input(*bkey);

            if(nullptr != bp) {

                if(bp->bath_input_type() == trn::BT_DVL)
                {
                    beam_geometry *bgeo = xpp->lookup_geo(*bath_0_key, trn::BT_DVL);
                    dvlgeo *geo = static_cast<dvlgeo *>(bgeo);

                    // compute MB1 beam components in vehicle frame
                    //                    trnx_utils::transform_dvl(bi, ai, geo, snd);
                    transform_dvl(bi, ai, geo, snd);

                } else if(bp->bath_input_type() == trn::BT_DELTAT)
                {
                    beam_geometry *bgeo = xpp->lookup_geo(*bath_0_key, trn::BT_DELTAT);
                    mbgeo *geo = static_cast<mbgeo *>(bgeo);

                    // compute MB1 beam components in vehicle frame
                    transform_deltat(bi, ai, geo, snd);

                } else {
                    fprintf(stderr,"%s:%d ERR - unsupported input_type[%d] beam transformation invalid\n", __func__, __LINE__, bp->bath_input_type());
                }
            } else {
                fprintf(stderr,"%s:%d ERR - NULL bath input; skipping transforms\n", __func__, __LINE__);
            }

            mb1_set_checksum(snd);

            // check modulus
            if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0){

                if(cfg->debug() >= TRNDL_PLUGIDT ){
                    fprintf(stderr,"%s - >>>>>>> Publishing MB1:\n",__func__);
                    mb1_show(snd, (cfg->debug()>=5 ? true: false), 5);
                }

                // publish MB1 to mbtrnpp
                ctx->pub_mb1(snd, xpp->pub_list(), cfg);

                if(ctx->trncli_count() > 0){

                    // publish poseT/measT to trn-server
                    int trn_type = bp->bath_input_type();

                    GeoCon gcon(ctx->utm_zone());
                    poseT *pt = trnx_utils::mb1_to_pose(snd, ai, NULL, &gcon);
                    measT *mt = trnx_utils::mb1_to_meas(snd, ai, trn_type, &gcon);

                    if(cfg->debug() >= TRNDL_PLUGIDT ){
                        fprintf(stderr,"%s - >>>>>>> Publishing POSE:\n",__func__);
                        trnx_utils::pose_show(*pt);
                        fprintf(stderr,"%s - >>>>>>> Publishing MEAS:\n",__func__);
                        trnx_utils::meas_show(*mt);
                    }


                    if(pt != nullptr && mt != nullptr){

                        double nav_time = ni->time_usec()/1e6;

                        // publish update TRN, publish estimate to TRN, LCM
                        ctx->pub_trn(nav_time, pt, mt, trn_type, xpp->pub_list(), cfg);
                    }

                    if(pt != nullptr)
                        delete pt;
                    if(mt != nullptr)
                        delete mt;
                }

            } else {
                TRN_NDPRINT(TRNDL_PLUGIDT, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
            }
            ctx->inc_cbcount();


            // write CSV
            if(ctx->write_mb1_csv(snd, bi, ai, vi) > 0){
                cfg->stats().mb_csv_n++;
            }

            ctx->write_mb1_bin(snd);

            retval=0;

            // release sounding memory
            mb1_destroy(&snd);
        }

        if(bi != nullptr)
            delete bi;
        if(ai != nullptr)
            delete ai;
        if(ni != nullptr)
            delete ni;
        if(vi != nullptr)
            delete vi;
    }

    return retval;
}
