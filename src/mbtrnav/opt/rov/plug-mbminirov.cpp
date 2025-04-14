#include "trnx_plugin.hpp"

// g++ -c -g -O2 -std=c++11 -I. -I../../utils -I../../qnx-utils -I../../terrain-nav -I../../newmat -I../../trnw -I/usr/local/include -I/opt/local/include -fPIC plug-mbminirov.cpp

// g++ -shared -o libdvl.so plug-idt.o -L../../bin -L/usr/local/lib -L/opt/local/lib -L. -lnewmat -lgeolib -ltrnw -lqnx -lmb1 -ltrn -ludpms -ltrncli -lnetcdf -llcm -ldvl

// Process Norbit multibeam sounding using
// miniROV and optionally Compas sled nav/attitude sensors.
// expects:
// b[0]   : vehicle bath (norbit WBMS)
// b[1]   : sled bath
// a[0]   : vehicle attitude
// a[1]   : sled attitude
// geo[0] : vehicle bath geometry
// geo[1] : sled bath geometry
// geo[2] : vehicle nav geometry
// geo[3] : sled nav geometry

int transform_mbminirov(trn::bath_info **bi, trn::att_info **ai, beam_geometry **bgeo, mb1_t *r_snd)
{
    int retval = -1;

    // validate inputs
    if(NULL == r_snd || NULL == ai|| NULL == bi || NULL == bgeo){
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] bgeo[%p] snd[%p]\n", __func__, bi, ai, bgeo, r_snd);
        return retval;
    }

    if(bgeo[0] == nullptr || bgeo[1] == nullptr){
        fprintf(stderr, "%s - geometry error : NULL input bgeo[%p] {%p, %p, %p} \n", __func__, bgeo, (bgeo?bgeo[0]:nullptr), (bgeo?bgeo[1]:nullptr), (bgeo?bgeo[2]:nullptr));
        return retval;
    }

    // map indexed inputs to readable names

    // 0: VEH DVL bath
    // 1: OIS DVL bath
    trn::bath_info *veh_bath = bi[0];
    trn::bath_info *ois_bath = bi[1];

    // 0: VEH ATT
    // 1: OIS ATT
    trn::att_info *veh_att= ai[0];
    trn::att_info *ois_att= ai[1];

    // 0: VEH DVL geometry
    // 1: OIS DVL geometry
    // 2: VEH NAV geometry
    // 2: OIS NAV geometry
    beam_geometry *veh_bathgeo = bgeo[0];
    beam_geometry *ois_bathgeo = bgeo[1];
    beam_geometry *veh_navgeo = bgeo[2];
    beam_geometry *ois_navgeo = bgeo[3];

    if(veh_bathgeo && static_cast<mbgeo *>(veh_bathgeo)->beam_count <= 0){
        fprintf(stderr, "%s - geometry warning : geo[0] beams <= 0 {%u}\n", __func__, static_cast<mbgeo *>(veh_bathgeo)->beam_count);
    }

    if(NULL == veh_att || NULL == veh_bathgeo){
        fprintf(stderr, "%s - ERR invalid info ai[0][%p] ai[1][%p] bi[0][%p] \n", __func__, veh_att, ois_att, veh_bathgeo);
        return retval;
    }

    // get beam components in reference sensor frame
    Matrix mBcompSF = trnx_utils::mb_sframe_components(veh_bath, static_cast<mbgeo *>(veh_bathgeo), 1.0);

    // vehicle attitude (relative to NED, radians)
    // r/p/y  (phi/theta/psi)
    // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
    double vATT[3] = {veh_att->roll(), veh_att->pitch(), 0.};
    // vehicle attitude (pitch, roll, heading(=0))
    Matrix mATT = trnx_utils::affine321Rotation(vATT);

    // BATH sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // wrt sensor mounted across track, b[0] port, downward facing
    double vBATH_ROT[3] = { veh_bathgeo->ro_u(0), veh_bathgeo->ro_u(1), veh_bathgeo->ro_u(2)};
    // Use transpose of sensor frame rotation
    // (passive rotation: rotate coordinate system, not vector)
    Matrix mBATH_ROT = trnx_utils::affine321Rotation(vBATH_ROT);

    // apply BATH sensor frame rotation, vehicle attitude transforms
    // to get (unscaled) beam components in vehicle frame, i.e. direction cosines
    Matrix mBcompVF = mATT.t() * mBATH_ROT.t() * mBcompSF;

    // adjust sounding depth (Z+ down)
    // should not be needed
    double zofs = (ois_navgeo == nullptr ? 0. : ois_navgeo->xmap["depthOfs"]);
    r_snd->depth += zofs;

    if(trn_debug::get()->debug() >= TRNDL_PLUGMBMINIROV){

        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s: --- \n",__func__);
        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "bath: [%p]\n", veh_bath);
        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "att: [%p %p]\n", ois_att, veh_att);
        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "geo: [%p]\n", bgeo);

        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "veh_bathgeo:\n%s\n", (veh_bathgeo == nullptr ? "n/a" : veh_bathgeo->tostring().c_str()));
        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "ois_navgeo:\n%s\n", (ois_navgeo == nullptr ? "n/a" : ois_navgeo->tostring().c_str()));

//        double PA[2] = { (ai[0] == nullptr ? 0. : Math::radToDeg(ai[0]->pitch())), (ai[1] == nullptr ? 0. : Math::radToDeg(ai[1]->pitch())) };
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "arm rotation (deg) Pv[%.3lf] Pa[%.3lf] angle[%.3lf]\n", PA[0], PA[1], Math::radToDeg(Wa));
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "Z ofs: (m) %.3lf\n", zofs);
//
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "Xo, Yo, Zo, Wo [%.3lf, %.3lf, %.3lf, %.3lf (%.3lf)]\n", Xo, Yo, Zo, Wo, RTD(Wo));
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "Xr, Yr, Zr, Wr [%.3lf, %.3lf, %.3lf, %.3lf (%.3lf)]\n", Xr, Yr, Zr, Wr, RTD(Wr));
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "dX, dY, dZ[%.3lf, %.3lf, %.3lf]\n", dX, dY, dZ);
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "r[%.3lf]\n", r);

//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "ARM_ROT  [%.3lf, %.3lf, %.3lf] [%.3lf, %.3lf, %.3lf] deg\n", vARM_ROT[0], vARM_ROT[1], vARM_ROT[2], RTD(vARM_ROT[0]), RTD(vARM_ROT[1]), RTD(vARM_ROT[2]));
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "NAV_ROT  [%.3lf, %.3lf, %.3lf] [%.3lf, %.3lf, %.3lf] deg\n", vNAV_ROT[0], vNAV_ROT[1], vNAV_ROT[2], RTD(vNAV_ROT[0]), RTD(vNAV_ROT[1]), RTD(vNAV_ROT[2]));
        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "BATH_ROT  [%.3lf, %.3lf, %.3lf] [%.3lf, %.3lf, %.3lf] deg\n", vBATH_ROT[0], vBATH_ROT[1], vBATH_ROT[2], RTD(vBATH_ROT[0]), RTD(vBATH_ROT[1]), RTD(vBATH_ROT[2]));
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "BATH_TRANo [%.3lf, %.3lf, %.3lf]\n", veh_bathgeo->tr_m(0), veh_bathgeo->tr_m(1), veh_bathgeo->tr_m(2));
//        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "BATH_TRAN [%.3lf, %.3lf, %.3lf]\n", vBATH_TRAN[0], vBATH_TRAN[1], vBATH_TRAN[2]);
        const char *pinv = (ai[0]->flags().is_set(trn::AF_INVERT_PITCH)? "(p-)" :"(p+)");
        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "VATT (RPH) [%.3lf, %.3lf, %.3lf] rad\n", vATT[0], vATT[1], vATT[2]);
        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "VATT (RPH) [%.3lf, %.3lf, %.3lf] deg %s hdg(%.3lf)\n",
                    Math::radToDeg(vATT[0]), Math::radToDeg(vATT[1]), Math::radToDeg(vATT[2]), pinv, Math::radToDeg(ai[0]->heading()));

        TRN_NDPRINT(TRNDL_PLUGMBMINIROV,"\n");
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

        if (urange != 0.) {

            // apply scale to vehicle frame components (beams_VF)
            // and transform coordinates to INS origin
            double vRange[3] = {urange, urange, urange};
            Matrix mRange = trnx_utils::affineScale(vRange);
            // beams in vehicle frame, before translation
            Matrix mBeams = mRange * mBcompVF;

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

        if(trn_debug::get()->debug() >= TRNDL_PLUGMBMINIROV){

            double range = sqrt(rho[0] * rho[0] + rho[1] * rho[1] + rho[2] * rho[2]);

            // calculated beam range (should match measured range)
            //double rho[3] = {r_snd->beams[idx[0]].rhox, r_snd->beams[idx[0]].rhoy, r_snd->beams[idx[0]].rhoz};
            double rhoNorm = trnx_utils::vnorm( rho );

            // calculate component angles wrt vehicle axes
            double axr = (rhoNorm == 0. ? 0. : acos(rho[0] / rhoNorm));
            double ayr = (rhoNorm == 0. ? 0. : acos(rho[1] / rhoNorm));
            double azr = (rhoNorm == 0. ? 0. : acos(rho[2] / rhoNorm));

            TRN_NDPRINT(TRNDL_PLUGMBMINIROV_H, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.4lf] rhoy[%7.4lf] rhoz[%7.4lf]     ax[%6.3lf] ay[%6.3lf] az[%6.3lf]\n",
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
    TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s: --- \n\n",__func__);

    retval = 0;
    return retval;
}

// Process TRN for Norbit multibeam on miniROV with Compas Sled.
// expects:
// b[0]   : vehicle bath
// b[1]   : sled bath
// a[0]   : vehicle attitude
// a[1]   : sled attitude
// n[0]   : vehicle nav
// n[1]   : sled nav
// d[0]   : optional alternative depth sensor
// v[0]   : optional velocity
// geo[0] : veh bath geo
// geo[1] : veh nav geo
int cb_proto_mbminirov(void *pargs)
{
    int retval=-1;

    TRN_NDPRINT(TRNDL_PLUGMBMINIROV_H, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

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
        if(ctx == nullptr || !ctx->has_callback("cb_proto_mbminirov"))
        {
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        std::string *bkey[2] = {ctx->bath_input_chan(0), ctx->bath_input_chan(1)};
        std::string *akey[2] = {ctx->att_input_chan(0), ctx->att_input_chan(1)};
        std::string *nkey[2] = {ctx->nav_input_chan(0), ctx->nav_input_chan(1)};
        std::string *vkey[1] = {ctx->vel_input_chan(0)};
        std::string *dkey[1] = {ctx->depth_input_chan(0)};

        // vi, ni[1], ai[1] is optional
        if(bkey[0] == nullptr || nkey[0] == nullptr || akey[0] == nullptr)
        {
            ostringstream ss;
            ss << (bkey[0]==nullptr ? " bkey[0]" : "");
            ss << (akey[0]==nullptr ? " akey[0]" : "");
            ss << (akey[1]==nullptr ? " akey[1]" : "");
            ss << (nkey[0]==nullptr ? " nkey[0]" : "");
            ss << (nkey[1]==nullptr ? " nkey[1]" : "");
            ss << (nkey[1]==nullptr ? " dkey[1]" : "");
            ss << (vkey[0] == nullptr ? " vkey[0]" : "");
            TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s:%d ERR - NULL input key: %s\n", __func__, __LINE__, ss.str().c_str());
                continue;
        }

        trn::bath_info *bi[2] = {xpp->get_bath_info(*bkey[0]), (bkey[1] == nullptr ? nullptr : xpp->get_bath_info(*bkey[1]))};
        trn::nav_info *ni[2] = {xpp->get_nav_info(*nkey[0]), (nkey[1] == nullptr ? nullptr : xpp->get_nav_info(*nkey[1]))};
        trn::att_info *ai[2] = {xpp->get_att_info(*akey[0]), (akey[1] == nullptr ? nullptr : xpp->get_att_info(*akey[1]))};
        trn::depth_info *di[1] = {(dkey[0] == nullptr? nullptr : xpp->get_depth_info(*dkey[0]))};
        trn::vel_info *vi[1] = {(vkey[0] == nullptr ? nullptr : xpp->get_vel_info(*vkey[0]))};

        // bi[1], ai[1], ni[1], vi, di optional
        if(bi[0] == nullptr || ni[0] == nullptr || ai[0] == nullptr)
        {
            ostringstream ss;
            ss << (bi[0] == nullptr ? " bi[0]" : "");
            ss << (ai[0] == nullptr ? " ai[0]" : "");
            ss << (ai[1] == nullptr ? " ai[1]" : "");
            ss << (ni[0] == nullptr ? " ni[0]" : "");
            ss << (ni[1] == nullptr ? " ni[1]" : "");
            ss << (vi[0] == nullptr ? " vi[0]" : "");
            TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s:%d WARN - NULL info instance: %s\n", __func__, __LINE__, ss.str().c_str());
            continue;
        }

        if(bkey[0] != nullptr && bi[0] != nullptr)
            TRN_NDPRINT(TRNDL_PLUGMBMINIROV_H, "BATHINST.%s : %s\n",bkey[0]->c_str(), bi[0]->bathstr());
        if(bkey[1] != nullptr && bi[1] != nullptr)
            TRN_NDPRINT(TRNDL_PLUGMBMINIROV_H, "BATHINST.%s : %s\n",bkey[1]->c_str(), bi[1]->bathstr());

        trn::bath_info *veh_bi = bi[0];
        trn::bath_info *ois_bi = bi[1];
        trn::att_info *veh_att = ai[0];
        trn::att_info *ois_att = ai[1];
        trn::nav_info *veh_nav = ni[0];
        trn::nav_info *ois_nav = ni[1];
        trn::depth_info *opt_dep = di[0];
        trn::vel_info *opt_vel = vi[0];

        trn::bath_info *snd_bath = nullptr;
        trn::att_info *snd_att = nullptr;
        trn::nav_info *snd_nav = nullptr;

        bool use_vbath = (ctx->mUmap["USE_VBATH"] == 1 ? true : false);

        if(use_vbath) {
            // use vehicle bath
            snd_bath = veh_bi;
            snd_att = veh_att;
            snd_nav = veh_nav;
        } else {
            // use sled bath
            snd_bath = ois_bi;
            snd_att = ois_att;
            snd_nav = ois_nav;
        }

        // sled DVL beam count
        size_t n_beams = snd_bath->beam_count();

        if (n_beams > 0) {

            // generate MB1 sounding (raw beams)
            mb1_t *snd = trnx_utils::lcm_to_mb1(snd_bath, snd_nav, snd_att);


            std::list<trn::beam_tup> beams = snd_bath->beams_raw();
            std::list<trn::beam_tup>::iterator it;

            // if streams_ok, bs/bp pointers have been validated
            trn::bath_input *bp[2] = {xpp->get_bath_input(*bkey[0]), xpp->get_bath_input(*bkey[1])};
            int trn_type[4] = {-1, -1, trn::BT_NONE, trn::BT_NONE};

            if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0) {


                double alt_depth = -1.0;
                if(snd_nav != NULL) {
                    alt_depth = opt_dep->pressure_to_depth_m(snd_nav->lat());
                    TRN_NDPRINT(3, "ni depth: %.3lf di pressure: %.3lf lat: %.3lf alt_depth: %.3lf\n", snd_nav->depth(), opt_dep->pressure_dbar(), snd_nav->lat(), alt_depth );
                    // trn::depth_input *din = xpp->get_depth_input(*dkey[0]);
                    // TRN_NDPRINT(3, "presssure:\n");
                    // din->tostream(std::cerr);
                    snd->depth = alt_depth;
                }

                // log raw beams
                ctx->write_rawbath_csv(snd_bath, snd_nav, snd_att, opt_vel, ctx->utm_zone(), alt_depth);

                if(nullptr != bp[0]) {
                    trn_type[0] = bp[0]->bath_input_type();

                    if(trn_type[0] == trn::BT_MULTIBEAM)
                    {
                        beam_geometry *bgeo[3] = {nullptr, nullptr, nullptr};

                        // get geometry for bath, veh nav, sled nav
                        bgeo[0] = xpp->lookup_geo(*bkey[0], trn_type[0]);
                        bgeo[1] = xpp->lookup_geo(*bkey[1], trn_type[1]);
                        bgeo[2] = xpp->lookup_geo(*nkey[0], trn_type[2]);
                        bgeo[2] = (nkey[1] == nullptr ? nullptr : xpp->lookup_geo(*nkey[1], trn_type[3]));
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
                        if (transform_mbminirov(bi, ai, bgeo, snd) != 0) {
                            TRN_NDPRINT(TRNDL_PLUGMBMINIROV_H, "%s:%d ERR - transform_mbminirov failed\n", __func__, __LINE__);
                            cfg->stats().err_plugin_n++;
                            continue;
                        }
                        TrnxPlugin::adjust_mb1_nav_fixed(ai, bgeo, ctx->geocon(), snd);
                    } else {
                        fprintf(stderr,"%s:%d ERR - unsupported input_type[%d] beam transformation invalid\n", __func__, __LINE__, trn_type[0]);
                    }
                } else {
                    fprintf(stderr,"%s:%d ERR - NULL bath input; skipping transforms\n", __func__, __LINE__);
                }
                mb1_set_checksum(snd);

                TRN_NDPRINT(3, "%s - >>>>>>> Publishing MB1\n", __func__);
                mb1_show(snd, (cfg->debug()>=4 ? true: false), 5);
//                if(cfg->debug() >= TRNDL_PLUGMBMINIROV ){
//                    mb1_show(snd, (cfg->debug()>=TRNDL_PLUGMBMINIROV ? true: false), 5);
//                }

                // publish MB1 to mbtrnpp
                ctx->pub_mb1(snd, xpp->pub_list(), cfg);

                if (ctx->trncli_count() > 0) {

                    // if TRN clients configured
                    // generate poseT/measT
                    // and publish to trn-server

                    GeoCon gcon(ctx->utm_zone());
                    poseT *pt = trnx_utils::mb1_to_pose(snd, ai[0], vi[0], &gcon);
                    measT *mt = trnx_utils::mb1_to_meas(snd, ai[0], trn_type[0], &gcon);

                    if (pt != nullptr && mt != nullptr) {

                        if (cfg->debug() >= TRNDL_PLUGMBMINIROV ) {
                            fprintf(stderr,"%s - >>>>>>> Publishing POSE:\n", __func__);
                            trnx_utils::pose_show(*pt);
                            fprintf(stderr,"%s - >>>>>>> Publishing MEAS:\n", __func__);
                            trnx_utils::meas_show(*mt);
                        }

                        double nav_time = ni[0]->time_usec()/1.e6;

                        // publish update TRN, publish estimate to TRN, LCM
                        ctx->pub_trn(nav_time, pt, mt, trn_type[0], xpp->pub_list(), cfg);
                    } else {
                        TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s - >>>>>>> skipping pub_trn pt[%p], mt[%p]:\n", __func__, pt, mt);
                    }

                    if (pt != nullptr)
                        delete pt;
                    if (mt != nullptr)
                        delete mt;
                } else {
                    TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s - >>>>>>> No TRN clients:\n", __func__);
                }

                // write CSV
                if (ctx->write_mb1_csv(snd, bi[0], ai[0], vi[0]) > 0) {
                    TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s - >>>>>>> wrote MB1 CSV\n",__func__);
                    cfg->stats().mb_csv_n++;
                }

                // write MB1 binary
                if (ctx->write_mb1_bin(snd) >= 0) {
                    TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s - >>>>>>> wrote MB1 bin\n",__func__);
                    cfg->stats().mb_log_mb1_n++;
                }

            } else {
                TRN_NDPRINT(TRNDL_PLUGMBMINIROV, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
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
