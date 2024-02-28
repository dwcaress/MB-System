#include "trnx_plugin.hpp"

// g++ -c -g -O2 -std=c++11 -I. -I../../utils -I../../qnx-utils -I../../terrain-nav -I../../newmat -I../../trnw -I/usr/local/include -I/opt/local/include -fPIC plug-idt.cpp

// g++ -shared -o libdvl.so plug-idt.o -L../../bin -L/usr/local/lib -L/opt/local/lib -L. -lnewmat -lgeolib -ltrnw -lqnx -lmb1 -ltrn -ludpms -ltrncli -lnetcdf -llcm -ldvl

// Transform IDT beams given nav/attitude
// mounted LASS tilting sled
// expects:
// b[0]   : vehicle bath (deltaT)
// a[0]   : vehicle attitude
// a[1]   : sled attitude
// n[0]   : vehicle navigation
// n[1]   : sled navigation
// geo[0] : mbgeo (multibeam geometry)
// geo[1] : txgeo (sled INS geometry)
// geo[2] : txgeo (veh nav geometry)

int transform_idtlass(trn::bath_info **bi, trn::att_info **ai, beam_geometry **bgeo, mb1_t *r_snd)
{
    int retval = -1;

    // validate inputs
    if(NULL == bgeo || bgeo[0] == nullptr || bgeo[1] == nullptr || bgeo[2] == nullptr){
        fprintf(stderr, "%s - geometry error : NULL input bgeo[%p] {%p, %p, %p} \n", __func__, bgeo, (bgeo?bgeo[0]:nullptr), (bgeo?bgeo[1]:nullptr), (bgeo?bgeo[2]:nullptr));
        return retval;
    }

    // IDT geometry
    mbgeo *mb_geo[1] = {static_cast<mbgeo *>(bgeo[0])};

    // 0: sled INS geometry
    // 1: veh nav geometry
    txgeo *tx_geo[2] = {static_cast<txgeo *>(bgeo[1]), static_cast<txgeo *>(bgeo[2])};

    if(mb_geo[0] && mb_geo[0]->beam_count <= 0){
        fprintf(stderr, "%s - geometry warning : geo[0] beams <= 0 {%u}\n", __func__, mb_geo[0]->beam_count);
    }

    if(NULL == r_snd || NULL == ai|| NULL == bi){
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
        return retval;
    }

    if(NULL == ai[0] || NULL == ai[1] || NULL == bi[0]){
        fprintf(stderr, "%s - ERR invalid info ai[0][%p] ai[1][%p] bi[0][%p] \n", __func__, ai[0], ai[1], bi[0]);
        return retval;
    }

    Matrix mBcompSF = trnx_utils::mb_sframe_components(bi[0], mb_geo[0], 1.0);

    // vehicle attitude (relative to NED, radians)
    // r/p/y  (phi/theta/psi)
    // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
    double vATT[3] = {ai[0]->roll(), ai[0]->pitch(), 0.};

#ifdef CODE_20231010
    // sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // wrt sensor mounted across track, b[0] port, downward facing
    double SROT[3] = { DTR(mb_geo[0]->svr_deg[0]), DTR(mb_geo[0]->svr_deg[1]), DTR(mb_geo[0]->svr_deg[2])};

    // sensor mounting translation offsets (relative to vehicle CRP, meters)
    // +x: fwd +y: stbd, +z:down
    // or should it be relative to location sensor?
    double STRN[3] = {mb_geo[0]->svt_m[0], mb_geo[0]->svt_m[1], mb_geo[0]->svt_m[2]};

    // 2023/10/10 : first order
    // - uncompensated lat/lon from sled kearfott
    // - use vehicle depth ()

    // mounting rotation matrix
    Matrix mat_SROT = trnx_utils::affine321Rotation(SROT);
    // mounting translation matrix
    Matrix mat_STRN = trnx_utils::affineTranslation(STRN);
    // vehicle attitude (pitch, roll, heading)
    Matrix mat_VATT = trnx_utils::affine321Rotation(VATT);

    // combine to get composite tranformation
    // order is significant:

    // apply IDT mounting translation, rotation
    Matrix S0 = mat_SROT * mat_STRN;
    // apply vehicle attitude
    Matrix S1 = mat_VATT * S0;

    // apply coordinate transforms
    Matrix beams_VF = S1 * beams_SF;

    // snd is intialized with vehicle nav
    // if initialized w/ sled INS
    // adjust depth for mounting offset difference (Z+ down)
    // Znav - Zmb
    r_snd->depth += (tx_geo[0]->tran_m[3] - tx_geo[1]->tran_m[3]);


#endif


    double Xo = tx_geo[0]->tran_m[0];
    double Yo = tx_geo[0]->tran_m[1];
    double Zo = tx_geo[0]->tran_m[2];
    double r = sqrt(Xo*Xo + Zo*Zo);
    double Wo = atan2(Zo, Xo);
    double Wa = ai[1]->pitch() - ai[0]->pitch();
    double Wr = (Wo + Wa);
    double Xr = r * cos(Wr);
    double Yr = Yo;
    double Zr = r * sin(Wr);
    // INS position offsets due to arm rotation
    double dX = Xr - Xo;
    double dY = Yr - Yo;
    double dZ = Zo - Zr;

    // arm rotation matrix
    double vARM_ROT[3] = {0., -Wa, 0.};
    Matrix mat_ARMROT = trnx_utils::affine321Rotation(vARM_ROT);

    // INS mount rotation matrix
    double vINS_ROT[3] = {DTR(tx_geo[0]->rot_deg[0]), DTR(tx_geo[0]->rot_deg[1]), DTR(tx_geo[0]->rot_deg[2])};
    Matrix mINSROT = trnx_utils::affine321Rotation(vINS_ROT);

    // sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // wrt sensor mounted across track, b[0] port, downward facing
    double vIDT_ROT[3] = { DTR(mb_geo[0]->svr_deg[0]), DTR(mb_geo[0]->svr_deg[1]), DTR(mb_geo[0]->svr_deg[2])};
    // Use transpose of sensor frame rotation
    // (passive rotation: rotate coordinate system, not vector)
    Matrix mIDT_ROT = trnx_utils::affine321Rotation(vIDT_ROT);

    // sensor mounting translation offsets (relative to INS in vehicle frame, meters)
    // +x: fwd +y: stbd, +z:down
    double vIDT_TRAN[3] = { -(mb_geo[0]->svt_m[0] + dX), -(mb_geo[0]->svt_m[1] + dY), -(mb_geo[0]->svt_m[2] + dZ)};
    Matrix mIDT_TRAN = trnx_utils::affineTranslation(vIDT_TRAN);

    // vehicle attitude (pitch, roll, heading(=0))
    Matrix mATT = trnx_utils::affine321Rotation(vATT);

    // apply IDT sensor frame rotation, vehicle attitude transforms
    // to get (unscaled) beam components in vehicle frame, i.e. direction cosines
    Matrix mBcompVF = mATT.t() * mIDT_ROT.t() * mBcompSF;

    // adjust sounding depth (Z+ down)
    // should not be needed
    double zofs = tx_geo[0]->xmap["depthOfs"];
    r_snd->depth += zofs;

    if(trn_debug::get()->debug() >= TRNDL_PLUGIDTLASS){
        
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s: --- \n",__func__);
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "mb_geo:\n%s\n",mb_geo[0]->tostring().c_str());
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "tx_geo.0:\n%s\n",tx_geo[0]->tostring().c_str());
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "tx_geo.1:\n%s\n",tx_geo[1]->tostring().c_str());
        
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "arm rotation (deg) Pv[%.3lf] Pa[%.3lf] angle[%.3lf]\n", Math::radToDeg(ai[0]->pitch()), Math::radToDeg(ai[1]->pitch()), Math::radToDeg(Wa));
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "Z ofs: (m) %.3lf\n", zofs);
        
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "Xo, Yo, Zo, Wo [%.3lf, %.3lf, %.3lf, %.3lf (%.3lf)]\n", Xo, Yo, Zo, Wo, RTD(Wo));
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "Xr, Yr, Zr, Wr [%.3lf, %.3lf, %.3lf, %.3lf (%.3lf)]\n", Xr, Yr, Zr, Wr, RTD(Wr));
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "dX, dY, dZ[%.3lf, %.3lf, %.3lf]\n", dX, dY, dZ);
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "r[%.3lf]\n", r);
        
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "ARM_ROT  [%.3lf, %.3lf, %.3lf] [%.3lf, %.3lf, %.3lf] deg\n", vARM_ROT[0], vARM_ROT[1], vARM_ROT[2], RTD(vARM_ROT[0]), RTD(vARM_ROT[1]), RTD(vARM_ROT[2]));
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "INS_ROT  [%.3lf, %.3lf, %.3lf] [%.3lf, %.3lf, %.3lf] deg\n", vINS_ROT[0], vINS_ROT[1], vINS_ROT[2], RTD(vINS_ROT[0]), RTD(vINS_ROT[1]), RTD(vINS_ROT[2]));
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "IDT_ROT  [%.3lf, %.3lf, %.3lf] [%.3lf, %.3lf, %.3lf] deg\n", vIDT_ROT[0], vIDT_ROT[1], vIDT_ROT[2], RTD(vIDT_ROT[0]), RTD(vIDT_ROT[1]), RTD(vIDT_ROT[2]));
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "IDT_TRANo [%.3lf, %.3lf, %.3lf]\n", mb_geo[0]->svt_m[0], mb_geo[0]->svt_m[1], mb_geo[0]->svt_m[2]);
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "IDT_TRAN [%.3lf, %.3lf, %.3lf]\n", vIDT_TRAN[0], vIDT_TRAN[1], vIDT_TRAN[2]);
            const char *pinv = (ai[0]->flags().is_set(trn::AF_INVERT_PITCH)? "(p-)" :"(p+)");
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "VATT     [%.3lf, %.3lf, %.3lf] rad\n", vATT[0], vATT[1], vATT[2]);
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "VATT     [%.2lf, %.2lf, %.2lf] deg %s hdg(%.2lf)\n",
                        Math::radToDeg(vATT[0]), Math::radToDeg(vATT[1]), Math::radToDeg(vATT[2]), pinv, Math::radToDeg(ai[0]->heading()));
        
            TRN_NDPRINT(TRNDL_PLUGIDTLASS,"\n");
    }
    // fill in the MB1 record using transformed beams
    std::list<trn::beam_tup> beams = bi[0]->beams_raw();
    std::list<trn::beam_tup>::iterator it;

    int idx[2] = {0, 1};
    for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
    {
        // get beam tuple: range, vehicle frame components (direction cosines)
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);

        // beam number (0-indexed)
        int b = std::get<0>(bt);

        double urange = std::get<1>(bt);
        r_snd->beams[idx[0]].beam_num = b;
        double rho[3] = {0., 0., 0.};
        double urho[3] = {0., 0., 0.};

        if (urange != 0.) {

            // apply scale to vehicle frame components (beams_VF)
            // and transform coordinates to INS origin
            double vRange[3] = {urange, urange, urange};
            Matrix mRange = trnx_utils::affineScale(vRange);
            // beams in vehicle frame, before translation
            Matrix mUBeams = mRange * mBcompVF;
            urho[0] = mUBeams(1, idx[1]);
            urho[1] = mUBeams(2, idx[1]);
            urho[2] = mUBeams(3, idx[1]);

            Matrix mBeams = mIDT_TRAN * mUBeams;

            // populate sounding with beam components
            // matrix row/col (1 indexed)
            r_snd->beams[idx[0]].rhox = mBeams(1, idx[1]);
            r_snd->beams[idx[0]].rhoy = mBeams(2, idx[1]);
            r_snd->beams[idx[0]].rhoz = mBeams(3, idx[1]);


            rho[0] = mBeams(1, idx[1]);
            rho[1] = mBeams(2, idx[1]);
            rho[2] = mBeams(3, idx[1]);

        } else {
            r_snd->beams[idx[0]].rhox = 0.;
            r_snd->beams[idx[0]].rhoy = 0.;
            r_snd->beams[idx[0]].rhoz = 0.;
        }

        if(trn_debug::get()->debug() >= TRNDL_PLUGIDTLASS){

            double range = sqrt(rho[0] * rho[0] + rho[1] * rho[1] + rho[2] * rho[2]);

           // double urange = sqrt(urho[0]*urho[0] + urho[1]*urho[1] + urho[2]*urho[2]);
//            double urhoNorm = trnx_utils::vnorm(urho);
//            double uaxr = (urhoNorm == 0. ? 0. : acos(urho[0] / urhoNorm));
//            double uayr = (urhoNorm == 0. ? 0. : acos(urho[1] / urhoNorm));
//            double uazr = (urhoNorm == 0. ? 0. : acos(urho[2] / urhoNorm));

//            TRN_NDPRINT(TRNDL_PLUGIDTLASS_H, "%s: urho[%7.4lf, %7.4lf, %7.4lf] urange[%7.4lf]\n",
//                        __func__, urho[0], urho[1], urho[2], urange);
//
//            TRN_NDPRINT(TRNDL_PLUGIDTLASS_H, "%s:  rho[%7.4lf, %7.4lf, %7.4lf] range [%7.4lf]\n",
//                        __func__, rho[0], rho[1], rho[2], range);

//            TRN_NDPRINT(TRNDL_PLUGIDTLASS_H, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.4lf] rhoy[%7.4lf] rhoz[%7.4lf]     ax[%6.3lf] ay[%6.3lf] az[%6.3lf] U\n",
//                        __func__, b, urange, urhoNorm,
//                        urho[0],
//                        urho[1],
//                        urho[2],
//                        Math::radToDeg(uaxr),
//                        Math::radToDeg(uayr),
//                        Math::radToDeg(uazr)
//                        );

            // calculated beam range (should match measured range)
            //double rho[3] = {r_snd->beams[idx[0]].rhox, r_snd->beams[idx[0]].rhoy, r_snd->beams[idx[0]].rhoz};
            double rhoNorm = trnx_utils::vnorm( rho );

            // calculate component angles wrt vehicle axes
            double axr = (rhoNorm == 0. ? 0. : acos(rho[0] / rhoNorm));
            double ayr = (rhoNorm == 0. ? 0. : acos(rho[1] / rhoNorm));
            double azr = (rhoNorm == 0. ? 0. : acos(rho[2] / rhoNorm));

            TRN_NDPRINT(TRNDL_PLUGIDTLASS_H, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.4lf] rhoy[%7.4lf] rhoz[%7.4lf]     ax[%6.3lf] ay[%6.3lf] az[%6.3lf]\n",
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
    TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s: --- \n\n",__func__);

    retval = 0;
    return retval;
}

// OI Toolsled
// vehicle: octans, IDT (Imagenex DeltaT)
// sled: kearfott
// expects:
// b[0]   : vehicle bath (deltaT)
// a[0]   : vehicle attitude
// a[1]   : sled attitude
// n[0]   : vehicle navigation
// n[1]   : sled navigation
// geo[0] : mbgeo
// geo[1] : oigeo
int cb_proto_idtlass(void *pargs)
{
    int retval=-1;

    TRN_NDPRINT(TRNDL_PLUGIDTLASS_H, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

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
        if(ctx == nullptr || !ctx->has_callback("cb_proto_idtlass"))
        {
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        int err_count = 0;

        std::string *bkey[1] = {ctx->bath_input_chan(0)};
        std::string *nkey[2] = {ctx->nav_input_chan(0),ctx->nav_input_chan(1)};
        std::string *akey[2] = {ctx->att_input_chan(0), ctx->att_input_chan(1)};
        std::string *vkey[1] = {ctx->vel_input_chan(0)};
        std::string *dkey[1] = {ctx->depth_input_chan(0)};

        // vi is optional
        if(bkey[0] == nullptr || nkey[0] == nullptr || nkey[1] == nullptr || akey[0] == nullptr || akey[1] == nullptr)
        {
            ostringstream ss;
            ss << (bkey[0]==nullptr ? " bkey[0]" : "");
            ss << (akey[0]==nullptr ? " akey[0]" : "");
            ss << (akey[1]==nullptr ? " akey[1]" : "");
            ss << (nkey[0]==nullptr ? " nkey[0]" : "");
            ss << (nkey[1]==nullptr ? " nkey[1]" : "");
            ss << (nkey[1]==nullptr ? " dkey[1]" : "");
            ss << (vkey[0] == nullptr ? " vkey[0]" : "");
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s:%d ERR - NULL input key: %s\n", __func__, __LINE__, ss.str().c_str());
                err_count++;
                continue;
        }

        trn::bath_info *bi[2] = {xpp->get_bath_info(*bkey[0]), nullptr};
        trn::nav_info *ni[2] = {xpp->get_nav_info(*nkey[0]), xpp->get_nav_info(*nkey[1])};
        trn::att_info *ai[2] = {xpp->get_att_info(*akey[0]), xpp->get_att_info(*akey[1])};
        trn::depth_info *di[1] = {xpp->get_depth_info(*dkey[0])};
        trn::vel_info *vi[1] = {(vkey[0] == nullptr ? nullptr : xpp->get_vel_info(*vkey[0]))};

        // vi optional
        if(bi[0] == nullptr || ni[0] == nullptr || ni[1] == nullptr || ai[1] == nullptr || ai[1] == nullptr)
        {
            ostringstream ss;
            ss << (bi[0] == nullptr ? " bi[0]" : "");
            ss << (ai[0] == nullptr ? " ai[0]" : "");
            ss << (ai[1] == nullptr ? " ai[1]" : "");
            ss << (ni[0] == nullptr ? " ni[0]" : "");
            ss << (ni[1] == nullptr ? " ni[1]" : "");
            ss << (vi[0] == nullptr ? " vi[0]" : "");
            TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s:%d WARN - NULL info instance: %s\n", __func__, __LINE__, ss.str().c_str());
                err_count++;
                continue;
        }

        if(bkey[0] != nullptr && bi[0] != nullptr)
            TRN_NDPRINT(TRNDL_PLUGIDTLASS_H, "BATHINST.%s : %s\n",bkey[0]->c_str(), bi[0]->bathstr());

        size_t n_beams = bi[0]->beam_count();

        if (n_beams > 0) {

            // generate MB1 sounding (raw beams)
            mb1_t *snd = trnx_utils::lcm_to_mb1(bi[0], ni[1], ai[0]);


            std::list<trn::beam_tup> beams = bi[0]->beams_raw();
            std::list<trn::beam_tup>::iterator it;

            // if streams_ok, bs/bp pointers have been validated
            trn::bath_input *bp[1] = {xpp->get_bath_input(*bkey[0])};
            int trn_type[3] = {-1, trn::BT_NONE, trn::BT_NONE};

            if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0) {


                double alt_depth = -1.0;
                if(di[0] != NULL) {
                    alt_depth = di[0]->pressure_to_depth_m(ni[1]->lat());
                    TRN_NDPRINT(3, "ni depth: %.3lf di pressure: %.3lf lat: %.3lf alt_depth: %.3lf\n", ni[1]->depth(), di[0]->pressure_dbar(), ni[1]->lat(), alt_depth );
                    //                trn::depth_input *din = xpp->get_depth_input(*dkey[0]);
                    //                TRN_NDPRINT(3, "presssure:\n");
                    //                din->tostream(std::cerr);
                    snd->depth = alt_depth;
                }

                // log raw beams
                ctx->write_rawbath_csv(bi[0], ni[1], ai[0], vi[0], ctx->utm_zone(), alt_depth);

                if(nullptr != bp[0]) {
                    trn_type[0] = bp[0]->bath_input_type();

                    if(trn_type[0] == trn::BT_DELTAT)
                    {
                        beam_geometry *bgeo[3] = {nullptr, nullptr};

                        // get geometry for IDT, sled INS, veh nav
                        bgeo[0] = xpp->lookup_geo(*bkey[0], trn_type[0]);
                        bgeo[1] = xpp->lookup_geo(*nkey[1], trn_type[1]);
                        bgeo[2] = xpp->lookup_geo(*nkey[0], trn_type[2]);
                        double t[6] = {
                            (bi[0] != NULL ? bi[0]->time_usec()/1.e6 : 0.),
                            (ni[0] != NULL ? ni[0]->time_usec()/1.e6 : 0.),
                            (ni[1] != NULL ? ni[1]->time_usec()/1.e6 : 0.),
                            (ai[0] != NULL ? ai[0]->time_usec()/1.e6 : 0.),
                            (ai[1] != NULL ? ai[1]->time_usec()/1.e6 : 0.),
                            (di[0] != NULL ? di[1]->time_usec()/1.e6 : 0.)
                        };

                        TRN_NDPRINT(3, "time skew (rel to bathy)\n");
                        TRN_NDPRINT(3, "bi[0] time: %.3lf\n", t[0]);
                        TRN_NDPRINT(3, "ni[0] time: %.3lf (%.3lf)\n", t[1], t[1]-t[0]);
                        TRN_NDPRINT(3, "ni[1] time: %.3lf (%.3lf)\n",  t[2], t[2]-t[0]);
                        TRN_NDPRINT(3, "ai[0] time: %.3lf (%.3lf)\n",  t[3], t[3]-t[0]);
                        TRN_NDPRINT(3, "ai[1] time: %.3lf (%.3lf)\n",  t[4], t[4]-t[0]);
                        TRN_NDPRINT(3, "di[0] time: %.3lf (%.3lf)\n",  t[5], t[5]-t[0]);

                        // compute MB1 beam components in vehicle frame
                        if (transform_idtlass(bi, ai, bgeo, snd) != 0) {
                            TRN_NDPRINT(TRNDL_PLUGIDTLASS_H, "%s:%d ERR - transform_idtlass failed\n", __func__, __LINE__);
                            err_count++;
                            cfg->stats().err_plugin_n++;
                            continue;
                        }

                    } else {
                        fprintf(stderr,"%s:%d ERR - unsupported input_type[%d] beam transformation invalid\n", __func__, __LINE__, trn_type[0]);
                    }
                } else {
                    fprintf(stderr,"%s:%d ERR - NULL bath input; skipping transforms\n", __func__, __LINE__);
                }
                mb1_set_checksum(snd);

                TRN_NDPRINT(3, "%s - >>>>>>> Publishing MB1\n", __func__);
                mb1_show(snd, (cfg->debug()>=4 ? true: false), 5);
//                if(cfg->debug() >= TRNDL_PLUGIDTLASS ){
//                    mb1_show(snd, (cfg->debug()>=TRNDL_PLUGIDTLASS ? true: false), 5);
//                }

                // publish MB1 to mbtrnpp
                ctx->pub_mb1(snd, xpp->pub_list(), cfg);

                if (ctx->trncli_count() > 0) {

                    // if TRN clients configured
                    // generate poseT/measT
                    // and publish to trn-server

                    poseT *pt = trnx_utils::mb1_to_pose(snd, ai[0], vi[0], (long)ctx->utm_zone());

                    measT *mt = trnx_utils::mb1_to_meas(snd, ai[0], trn_type[0], (long)ctx->utm_zone());


                    if (pt != nullptr && mt != nullptr) {

                        if (cfg->debug() >= TRNDL_PLUGIDTLASS ) {
                            fprintf(stderr,"%s - >>>>>>> Publishing POSE:\n", __func__);
                            trnx_utils::pose_show(*pt);
                            fprintf(stderr,"%s - >>>>>>> Publishing MEAS:\n", __func__);
                            trnx_utils::meas_show(*mt);
                        }

                        double nav_time = ni[0]->time_usec()/1.e6;

                        // publish update TRN, publish estimate to TRN, LCM
                        ctx->pub_trn(nav_time, pt, mt, trn_type[0], xpp->pub_list(), cfg);
                    } else {
                        TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s - >>>>>>> skipping pub_trn pt[%p], mt[%p]:\n", __func__, pt, mt);
                    }

                    if (pt != nullptr)
                        delete pt;
                    if (mt != nullptr)
                        delete mt;
                } else {
                    TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s - >>>>>>> No TRN clients:\n", __func__);
                }

                // write CSV
                if (ctx->write_mb1_csv(snd, bi[0], ai[0], vi[0]) > 0) {
                    TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s - >>>>>>> wrote MB1 CSV\n",__func__);
                    cfg->stats().mb_csv_n++;
                }

                // write MB1 binary
                if (ctx->write_mb1_bin(snd) >= 0) {
                    TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s - >>>>>>> wrote MB1 bin\n",__func__);
                    cfg->stats().mb_log_mb1_n++;
                }

            } else {
                TRN_NDPRINT(TRNDL_PLUGIDTLASS, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
            }

            ctx->inc_cbcount();

            retval=0;

            // release sounding memory
            mb1_destroy(&snd);
        } else {
            cfg->stats().err_nobeams_n++;
        }

        if(bi[0] != nullptr)
            delete bi[0];
        if(ai[0] != nullptr)
            delete ai[0];
        if(ai[1] != nullptr)
            delete ai[1];
        if(ni[0] != nullptr)
            delete ni[0];
        if(ni[1] != nullptr)
            delete ni[1];
        if(vi[0] != nullptr)
            delete vi[0];
    }

    return retval;
}
