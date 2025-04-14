#include "trnx_plugin.hpp"

// g++ -c -g -O2 -std=c++11 -I. -I../../utils -I../../qnx-utils -I../../terrain-nav -I../../newmat -I../../trnw -I/usr/local/include -I/opt/local/include -fPIC plug-oisledx.cpp

// g++ -shared -o liboisledx.so plug-oisledx.o -L../../bin -L/usr/local/lib -L/opt/local/lib -L. -lnewmat -lgeolib -ltrnw -lqnx -lmb1 -ltrn -ludpms -ltrncli -lnetcdf -llcm


// process DVL sounding from ocean imaging (LASS) sled
// The DVL is mounted on rotating arm, coincident with NAV (INS)
// Since the DVL is coincident with the INS, N,E offset adjustment is not required.
// However, the bath beam angles and sounding depth (?) must be adjusted for arm rotation
// and vehicle attitude (pitch, roll only).
// expects:
// bi[0]  - VEH DVL bath
// bi[1]  - OIS DVL bath (xmap: BNx,BNy,BNz)
// ai[0]  - VEH att
// ai[1]  - OIS att
// geo[0] - VEH_DVL dvlgeo (vehicle origin) (xmap:depthOffset)
// geo[1] - OIS_DVL dvlgeo (vehicle origin)
// geo[2] - OIS_NAV txgeo (vehicle origin) (xmap: NAx,NAy,NAz)
// snd    - sounding (w. navigation in vehicle frame)
static void transform_oidvl(trn::bath_info **bi, trn::att_info **ai, beam_geometry **bgeo, mb1_t *r_snd)
{
    // validate inputs
    if(NULL == r_snd || NULL == ai|| NULL == bi || NULL == bgeo){
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] bgeo[%p] snd[%p]\n", __func__, bi, ai, bgeo, r_snd);
        return;
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
    // 2: OIS NAV geometry
    beam_geometry *veh_dvlgeo = bgeo[0];
    beam_geometry *ois_dvlgeo = bgeo[1];
    beam_geometry *ois_navgeo = bgeo[2];

    if(ois_dvlgeo && static_cast<dvlgeo *>(ois_dvlgeo)->beam_count <= 0){
        fprintf(stderr, "%s - geometry error :ois_dvlgeo beams <= 0 {%u}\n", __func__, static_cast<dvlgeo *>(ois_dvlgeo)->beam_count);
        return;
    }

    if(NULL == ois_att || NULL == veh_att || NULL == ois_bath){
        fprintf(stderr, "%s - ERR invalid info ois_att[%p] veh_att[%p] ois_bath[%p] \n", __func__, ois_att, veh_att, ois_bath);
        return;
    }

    // beam components in reference sensor frame (mounted center, across track)
    Matrix mBcompSF = trnx_utils::dvl_sframe_components(ois_bath, static_cast<dvlgeo *>(ois_dvlgeo));

    // compute translation offset of NAV (on sled arm) due to arm rotation
    sled_rofs_t sled_ofs = {0}, *pofs = &sled_ofs;
    trnx_utils::sled_nav_rot_offsets(ois_att, veh_att, ois_navgeo, pofs);

    // set up rotations due to mounting and arm rotation
    double BROT_SF[3] = {ois_dvlgeo->ro_u(0), ois_dvlgeo->ro_u(1), ois_dvlgeo->ro_u(2)};
    double AOTRAN_VO[3] = {pofs->Ax, pofs->Ay, pofs->Az};
    double VOTRAN_AO[3] = {-pofs->Ax, -pofs->Ay, -pofs->Az};
    double BROT_AO[3] = {0., pofs->Wa, 0.};
    double VROT_ATT[3] = {veh_att->roll(), veh_att->pitch(), 0.};

    Matrix mBathSVRot = trnx_utils::affine321Rotation(BROT_SF);
    Matrix mOaOvTran = trnx_utils::affineTranslation(AOTRAN_VO);
    Matrix mArmRotOa = trnx_utils::affine321Rotation(BROT_AO);
    Matrix mOvOaTran = trnx_utils::affineTranslation(VOTRAN_AO);
    Matrix mVehAtt = trnx_utils::affine321Rotation(VROT_ATT);

    // apply sensor mounting rotations
    Matrix S0 = mBathSVRot.t() * mBcompSF;
    // apply arm rotation (translate to origin, rotate, translate back
    Matrix S1 = mOvOaTran.t() * mArmRotOa.t() * mOaOvTran.t() * S0;
    // apply vehicle pitch, roll
    // to get rotated, unscaled beam components in vehicle frame
    Matrix mBcompVF = mVehAtt.t() * S1;

    // adjust sounding depth (Z+ down)
    double zofs = (ois_dvlgeo == nullptr ? 0. : ois_dvlgeo->xmap["depthOfs"]);
    r_snd->depth += zofs; // + pofs->dZ

    if(trn_debug::get()->debug() >= TRNDL_PLUGOIDVLX) {
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "%s: --- \n",__func__);
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "bath: [%p]\n", ois_bath);
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "att: [%p %p]\n", ois_att, veh_att);
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "geo: [%p]\n", bgeo);

        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "ois_dvlgeo:\n%s\n", (ois_dvlgeo == nullptr ? "n/a" : ois_dvlgeo->tostring().c_str()));
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "ois_navgeo:\n%s\n", (ois_navgeo == nullptr ? "n/a" : ois_navgeo->tostring().c_str()));

        double PA[2] = {0,0};
        PA[0] = (ois_att == nullptr ? 0. : ois_att->pitch(trn::PA_DEGREES));
        PA[1] = (veh_att == nullptr ? 0. : veh_att->pitch(trn::PA_DEGREES));

        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "arm rotation (deg) Pois[%.3lf] Pveh[%.3lf] Wa[%.3lf]\n", PA[0], PA[1], Math::radToDeg(pofs->Wa));
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "zofs: (m) %.3lf\n", zofs);

        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "Xo, Yo, Zo, Ro, Wo, [%.3lf, %.3lf, %.3lf, %.3lf (%.3lf)]\n", pofs->Xo, pofs->Yo, pofs->Zo, pofs->Ro, pofs->Wo, RTD(pofs->Wo));
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "Xr, Yr, Zr, Wr [%.3lf, %.3lf, %.3lf, %.3lf, %.3lf (%.3lf)]\n", pofs->Xr, pofs->Yr, pofs->Zr, pofs->Wr, RTD(pofs->Wr));
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "dX, dY, dZ[%.3lf, %.3lf, %.3lf]\n", pofs->dX, pofs->dY, pofs->dZ);

        const char *pinv = (veh_att->flags().is_set(trn::AF_INVERT_PITCH)? "(p-)" :"(p+)");
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "VROT_ATT (deg) [%.2lf, %.2lf, %.2lf] hdg (%.2lf) %s\n",
                    Math::radToDeg(VROT_ATT[0]), Math::radToDeg(VROT_ATT[1]), Math::radToDeg(VROT_ATT[2]), ois_att->heading(trn::PA_DEGREES), pinv);

        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "\n");
    }

    std::list<trn::beam_tup> beams = ois_bath->beams_raw();
    std::list<trn::beam_tup>::iterator it;

    // fill in the MB1 record using transformed beams
    // zero- and one-based indexs
    int idx[2] = {0, 1};
    for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
    {
        // write beam data to MB1 sounding
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);

        // beam number (0-indexed)
        int b = std::get<0>(bt);
        double urange = std::get<1>(bt);
        r_snd->beams[idx[0]].beam_num = b;

        double rho[3] = {0., 0., 0.};

        if(urange != 0) {
            // apply scale to vehicle frame components (mBcompVF)
            // and transform coordinates to INS origin
            double vRange[3] = {urange, urange, urange};
            Matrix mRange = trnx_utils::affineScale(vRange);

            // scaled beams in vehicle frame
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

        if(trn_debug::get()->debug() >= TRNDL_PLUGOIDVLX) {
            double range = sqrt(rho[0] * rho[0] + rho[1] * rho[1] + rho[2] * rho[2]);


            // calculated beam range (should match measured range)
            double rhoNorm = trnx_utils::vnorm( rho );

            // calculate component angles wrt vehicle axes
            double axr = (rhoNorm == 0. ? 0. : acos(rho[0] / rhoNorm));
            double ayr = (rhoNorm == 0. ? 0. : acos(rho[1] / rhoNorm));
            double azr = (rhoNorm == 0. ? 0. : acos(rho[2] / rhoNorm));

            TRN_NDPRINT(TRNDL_PLUGOIDVLX_H, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf]     ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
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
    TRN_NDPRINT(TRNDL_PLUGOIDVLX, "%s: --- \n\n",__func__);

    return;
}

// Process vehicle DVL (on ROV vehicle frame) sounding
// using from ocean imaging toolsled nav (INS, mounted on rotating arm)
// Since the DVL is on the vehicle and the INS is on a rotating arm,
// the DVL sounding lat/lon and depth must be adjusted for arm rotation
// and beam angles adjusted for vehicle attitude (pitch, roll only).
// NOTE: this use case is somewhat unlikely, since the sled blocks the
// ROV DVL in it's usual position. However, it is a useful analog for
// other ROV-mounted bathy sensors with LASS NAV (e.g. Imagenex DeltaT)
// expects:
// bi[0]  - VEH DVL bath
// bi[1]  - OIS DVL bath (xmap: BNx,BNy,BNz)
// ai[0]  - VEH att
// ai[1]  - OIS att
// geo[0] - VEH_DVL dvlgeo (vehicle origin) (xmap:depthOffset)
// geo[1] - OIS_DVL dvlgeo (vehicle origin)
// geo[2] - OIS_NAV txgeo (vehicle origin) (xmap: NAx,NAy,NAz)
// snd    - sounding (w. navigation in vehicle frame)
static void transform_rovdvl(trn::bath_info **bi, trn::att_info **ai, beam_geometry **bgeo, mb1_t *r_snd)
{
    // validate inputs
    if(NULL == r_snd || NULL == ai|| NULL == bi || NULL == bgeo) {
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] bgeo[%p] snd[%p]\n", __func__, bi, ai, bgeo, r_snd);
        return;
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
    // 2: OIS NAV geometry
    beam_geometry *veh_dvlgeo = bgeo[0];
    beam_geometry *ois_dvlgeo = bgeo[1];
    beam_geometry *ois_navgeo = bgeo[2];

    if(veh_dvlgeo && static_cast<dvlgeo *>(veh_dvlgeo)->beam_count <= 0) {
        fprintf(stderr, "%s - geometry error :veh_dvlgeo beams <= 0 {%u}\n", __func__, static_cast<dvlgeo *>(veh_dvlgeo)->beam_count);
        return;
    }

    if(NULL == ois_att || NULL == veh_att || NULL == veh_bath) {
        fprintf(stderr, "%s - ERR invalid info ois_att[%p] veh_att[%p]  veh_bath[%p]\n", __func__, ois_att, veh_att, veh_bath);
        return;
    }

    // beam components in reference sensor frame
    Matrix mBcompSF = trnx_utils::dvl_sframe_components(veh_bath, static_cast<dvlgeo *>(veh_dvlgeo));

    // compute translation offset of NAV (on sled arm) due to arm rotation
    sled_rofs_t sled_ofs = {0}, *pofs = &sled_ofs;
    trnx_utils::sled_nav_rot_offsets(ois_att, veh_att, ois_navgeo, pofs);

    // sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // sensor frame: TBD
    // passive rotation: rotate coordinates, not vectors (use transpose)
    double BROT_SF[3] = {veh_dvlgeo->ro_u(0), veh_dvlgeo->ro_u(1), veh_dvlgeo->ro_u(2)};

    // vehicle attitude (relative to NED)
    // r/p/y (phi/theta/psi)
    // MB1 only applies roll, pitch
    double VROT_ATT[3] = {veh_att->roll(), veh_att->pitch(), 0.};

    Matrix mBathSVRot = trnx_utils::affine321Rotation(BROT_SF);
    Matrix mVehAtt = trnx_utils::affine321Rotation(VROT_ATT);

    // apply BATH sensor frame rotation, vehicle attitude transforms
    // to get (unscaled) beam components in vehicle frame, i.e. direction cosines
    Matrix mBcompVF = mVehAtt.t() * mBathSVRot.t() * mBcompSF;

    // adjust sounding depth (Z+ down)
    // should not be needed
    double zofs = (veh_dvlgeo == nullptr ? 0. : veh_dvlgeo->xmap["depthOfs"]);
    r_snd->depth += zofs;

    if(trn_debug::get()->debug() >= TRNDL_PLUGOIDVLX){
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "%s: --- \n",__func__);
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "bath: [%p]\n", veh_bath);
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "att: [%p %p]\n", ois_att, veh_att);
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "geo: [%p]\n", bgeo);

        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "veh_dvlgeo:\n%s\n", (veh_dvlgeo == nullptr ? "n/a" : veh_dvlgeo->tostring().c_str()));
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "ois_navgeo:\n%s\n", (ois_navgeo == nullptr ? "n/a" : ois_navgeo->tostring().c_str()));

        double PA[2] = {0,0};
        PA[0] = (ois_att == nullptr ? 0. : ois_att->pitch(trn::PA_DEGREES));
        PA[1] = (veh_att == nullptr ? 0. : veh_att->pitch(trn::PA_DEGREES));

        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "arm rotation (deg) Pois[%.3lf] Pveh[%.3lf] Wa[%.3lf]\n", PA[0], PA[1], Math::radToDeg(pofs->Wa));
        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "zofs: (m) %.3lf\n", zofs);

        TRN_NDPRINT(TRNDL_PLUGOIDVLX, "\n");
    }

    std::list<trn::beam_tup> beams = bi[1]->beams_raw();
    std::list<trn::beam_tup>::iterator it;

    // fill in the MB1 record using transformed beams
    // zero- and one-based indexs
    int idx[2] = {0, 1};
    for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
    {
        // write beam data to MB1 sounding
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);

        // beam number (0-indexed)
        int b = std::get<0>(bt);
        double urange = std::get<1>(bt);
        r_snd->beams[idx[0]].beam_num = b;

        double rho[3] = {0., 0., 0.};

        if(urange != 0) {
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

        if(trn_debug::get()->debug() >= TRNDL_PLUGOIDVLX) {
            double range = sqrt(rho[0] * rho[0] + rho[1] * rho[1] + rho[2] * rho[2]);


            // calculated beam range (should match measured range)
            double rhoNorm = trnx_utils::vnorm( rho );

            // calculate component angles wrt vehicle axes
            double axr = (rhoNorm == 0. ? 0. : acos(rho[0] / rhoNorm));
            double ayr = (rhoNorm == 0. ? 0. : acos(rho[1] / rhoNorm));
            double azr = (rhoNorm == 0. ? 0. : acos(rho[2] / rhoNorm));

            TRN_NDPRINT(TRNDL_PLUGOIDVLX_H, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf]     ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
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
    TRN_NDPRINT(TRNDL_PLUGOIDVLX, "%s: --- \n\n",__func__);

    return;
}
// input: OI sled DVL
// Context may provide umap value USE_VBATH to use
// VEH DVL with OIS nav. OTherwise, the OIS DVL is used.
// publish to: mbtrnpp , TRN server
// expects:
// bi[0]   : vehicle DVL bath
// bi[1]   : sled DVL bath
// ai[0]   : vehicle attitude
// ai[1]   : sled attitude
// ni[0]   : nav
// vi[0]   : vehicle velocity
int cb_proto_oisledx(void *pargs)
{
    int retval = -1;

    TRN_NDPRINT(3, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

    if(NULL == pargs) {
        return retval;
    }

    trn::trnxpp::callback_res_t *cb_res = static_cast<trn::trnxpp::callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;
    trnxpp_cfg *cfg = cb_res->cfg;

    cfg->stats().trn_cb_n++;

    // iterate over contexts
    std::vector<trn::trnxpp_ctx *>::iterator it;
    for(it = xpp->ctx_list_begin(); it != xpp->ctx_list_end(); it++) {

        trn::trnxpp_ctx *ctx = (*it);
        // if context defined for this callback
        if(ctx == nullptr || !ctx->has_callback("cb_proto_oisledx")) {
            TRN_TRACE();
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(5, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        std::string *bkey[2] = {ctx->bath_input_chan(0), ctx->bath_input_chan(1)};
        std::string *nkey[1] = {ctx->nav_input_chan(0)};
        std::string *akey[2] = {ctx->att_input_chan(0), ctx->att_input_chan(1)};
        std::string *vkey[1] = {ctx->vel_input_chan(0)};

        // vi is optional
        if(bkey[0] == nullptr || bkey[1] == nullptr || nkey[0] == nullptr || akey[0] == nullptr || akey[1] == nullptr) {
            ostringstream ss;
            ss << (bkey[0]==nullptr ? " bkey[0]" : "");
            ss << (bkey[1]==nullptr ? " bkey[1]" : "");
            ss << (akey[0]==nullptr ? " akey[0]" : "");
            ss << (akey[1]==nullptr ? " akey[1]" : "");
            ss << (nkey[0]==nullptr ? " nkey" : "");
            TRN_NDPRINT(5, "%s:%d WARN - NULL input key: %s\n", __func__, __LINE__, ss.str().c_str());
            continue;
        }

        trn::bath_info *bi[2] = {xpp->get_bath_info(*bkey[0]), xpp->get_bath_info(*bkey[1])};
        trn::nav_info *ni[1] = {xpp->get_nav_info(*nkey[0])};
        trn::att_info *ai[2] = {xpp->get_att_info(*akey[0]), xpp->get_att_info(*akey[1])};
        trn::vel_info *vi[1] = {(vkey[0] == nullptr ? nullptr : xpp->get_vel_info(*vkey[0]))};

        // vi optional
        if(bi[0] == nullptr || bi[1] == nullptr || ni[0] == nullptr || ai[1] == nullptr || ai[1] == nullptr || vi[0] == nullptr) {
            ostringstream ss;
            ss << (bi[0]==nullptr ? " bi[0]" : "");
            ss << (bi[1]==nullptr ? " bi[1]" : "");
            ss << (ai[0]==nullptr ? " ai[0]" : "");
            ss << (ai[1]==nullptr ? " ai[1]" : "");
            ss << (ni[0]==nullptr ? " ni[0]" : "");
            ss << (vi[0]==nullptr ? " vi[0]" : "");
            TRN_NDPRINT(5, "%s:%d WARN - NULL info instance: %s\n", __func__, __LINE__, ss.str().c_str());
        }

        if(bkey[0] != nullptr && bi[0] != nullptr)
            TRN_NDPRINT(6, "BATHINST.%s : %s\n",bkey[0]->c_str(), bi[0]->bathstr());
        if(bkey[1] != nullptr && bi[1] != nullptr)
            TRN_NDPRINT(6, "BATHINST.%s : %s\n",bkey[1]->c_str(), bi[1]->bathstr());

        trn::bath_info *veh_bi = bi[0];
        trn::bath_info *ois_bi = bi[1];
        trn::att_info *veh_att = ai[0];
        trn::att_info *ois_att = ai[1];
        trn::nav_info *sel_nav = ni[0];

        trn::bath_info *snd_bath = nullptr;
        trn::att_info *snd_att = nullptr;

        bool use_vbath = (ctx->mUmap["USE_VBATH"] == 1 ? true : false);

        if(use_vbath) {
            // use vehicle bath
            snd_bath = veh_bi;
            snd_att = veh_att;
        } else {
            // use sled bath
            snd_bath = ois_bi;
            snd_att = ois_att;
        }
        // sled DVL beam count
        size_t n_beams = snd_bath->beam_count();

        if(n_beams > 0) {

            // use configured bathy, nav, vehicle attitude
            mb1_t *snd = trnx_utils::lcm_to_mb1(snd_bath, sel_nav, snd_att);

            std::list<trn::beam_tup> beams = snd_bath->beams_raw();
            std::list<trn::beam_tup>::iterator it;

            // if streams_ok, bs/bp pointers have been validated
            trn::bath_input *bp[2] = {xpp->get_bath_input(*bkey[0]), xpp->get_bath_input(*bkey[1])};

            // TRN bath types (to lookup geometries)
            // 0 : veh bath type
            // 1 : sled bath type
            // 2 : sled nav geometry
            int trn_type[3] = {-1, -1, trn::BT_NONE};

            if(bp[0] != nullptr) {
                trn_type[0] = bp[0]->bath_input_type();
            }

            if(bp[1] != nullptr) {
                trn_type[1] = bp[1]->bath_input_type();
            }

            if(nullptr != snd_bath) {

                // 0: vehicle DVL geometry
                // 1: sled DVL geometry
                // 2: sled NAV geometry
                beam_geometry *bgeo[3] = {nullptr, nullptr, nullptr};

                if(nullptr != bp[0]) {
                    // geometry - VEH_DVL::Ov (vehicle origin)
                    bgeo[0] = xpp->lookup_geo(*bkey[0], trn_type[0]);
                }

                if(nullptr != bp[1]) {
                    // geometry - SLED_DVL::Ov (vehicle origin)
                    bgeo[1] = xpp->lookup_geo(*bkey[1], trn_type[1]);
                }

                 // geometry - SLED_NAV::Ov (vehicle origin)
                bgeo[2] = (nkey[0] == nullptr ? nullptr : xpp->lookup_geo(*nkey[0], trn_type[2]));

                if(ctx->mUmap["USE_VBATH"] != 0) {
                    transform_rovdvl(bi, ai, bgeo, snd);
                    trnx_utils::adjust_mb1_nav_rotating(ai, bgeo, ctx->geocon(), snd);
                } else {
                    // tranform oisled DVL beams
                    transform_oidvl(bi, ai, bgeo, snd);
                }

            } else {
                fprintf(stderr,"%s:%d ERR - NULL bath input; skipping transforms\n", __func__, __LINE__);
            }

            mb1_set_checksum(snd);

            // check modulus
            if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0) {

                if(cfg->debug() >= 4 ) {
                    mb1_show(snd, (cfg->debug()>=5 ? true: false), 5);
                }

                // publish MB1 to mbtrnpp
                ctx->pub_mb1(snd, xpp->pub_list(), cfg);


                if(ctx->trncli_count() > 0) {

                    // publish poseT/measT to trn-server
                    GeoCon gcon(ctx->utm_zone());
                    poseT *pt = trnx_utils::mb1_to_pose(snd, snd_att, NULL, &gcon);
                    measT *mt = trnx_utils::mb1_to_meas(snd, snd_att, trn_type[1], &gcon);

                    if(pt != nullptr && mt != nullptr) {

                        double nav_time = ni[0]->time_usec()/1e6;

                        // publish update TRN, publish estimate to TRN, LCM
                        ctx->pub_trn(nav_time, pt, mt, trn_type[1], xpp->pub_list(), cfg);
                    }

                    if(pt != nullptr)
                        delete pt;
                    if(mt != nullptr)
                        delete mt;
                }
            } else {
                TRN_NDPRINT(5, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
            }

            ctx->inc_cbcount();

            // write CSV
            // use sled bathy, vehicle attitude
            if(ctx->write_mb1_csv(snd, snd_bath, snd_att, vi[0]) > 0) {
                cfg->stats().mb_csv_n++;
            }

            // release sounding memory
            mb1_destroy(&snd);
            retval=0;
        }

        if(bi[0] != nullptr)
            delete bi[0];
        if(bi[1] != nullptr)
            delete bi[1];
        if(ai[0] != nullptr)
            delete ai[0];
        if(ai[1] != nullptr)
            delete ai[1];
        if(ni[0] != nullptr)
            delete ni[0];
        if(vi[0] != nullptr)
            delete vi[0];
    }

    return retval;
}
