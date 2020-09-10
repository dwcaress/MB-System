// variable declarations, initialization
var help_topics=[];
var TRN_RESON_HOST=[];
var TRN_HOST=[];
var TRN_LOGFILES=[];
var TRN_DATAFILES=[];
var TRN_MAPFILES=[];
var TRN_MBTRNDIR=[];
var mbhbt=[];
var trnhbt=[];
var trnuhbt=[];

// context-specific constants
var MBARI_LINUXVM_IP="134.89.13.19";
var CARSON_LINUXVM_IP="134.89.33.229";
var HOME_LINUXVM_IP="192.168.1.68";
var MBARI_WINVM_IP="134.89.13.X";
var CARSON_WINVM_IP="134.89.33.X";
var HOME_WINVM_IP="192.168.1.86";
var MAPPER1_RESON_IP="134.89.32.107";
var LINUXVM_IP_CURRENT=MBARI_LINUXVM_IP;
var WINVM_IP_CURRENT=MBARI_WINVM_IP;
var MBHBT_MAPPER1=0;
var MBHBT_TEST=15;
var TRNHBT_MAPPER1=0;
var TRNHBT_TEST=15;
var TRNUHBT_MAPPER1=0;
var TRNUHBT_TEST=15;

var RESON_LOGFILES="/cygdrive/d/cygwin64/logs/mbtrn";
var RESON_DATAFILES="/cygdrive/d/cygwin64/G2TerrainNav/config";
var RESON_MAPFILES="/cygdrive/d/cygwin64/maps";
var RESON_TRN_MBTRNDIR="/usr/local/bin"

var LINUXVM_LOGFILES="/home/headley/tmp/logs";
var LINUXVM_DATAFILES="/mnt/vmhost/config";
var LINUXVM_MAPFILES="/mnt/vmhost/maps";
var LINUXVM_TRN_MBTRNDIR="/mnt/vmhost/git/mbsys-trn/MB-System/src/mbtrnutils"

var WINVM_LOGFILES="/cygdrive/z/win_share/test/logs";
var WINVM_DATAFILES="/cygdrive/z/win_share/test/config";
var WINVM_MAPFILES="/cygdrive/z/win_share/test/maps";
var WINVM_TRN_MBTRNDIR="/home/headley/git/MB-System/src/mbtrnutils"

//initialize session
var SESSION=session_str();

// initialize presets
TRN_RESON_HOST["win.reson"]=MAPPER1_RESON_IP;
TRN_HOST["win.reson"]=MAPPER1_RESON_IP;
TRN_LOGFILES["win.reson"]=RESON_LOGFILES;
TRN_DATAFILES["win.reson"]=RESON_DATAFILES;
TRN_MAPFILES["win.reson"]=RESON_MAPFILES;
TRN_MBTRNDIR["win.reson"]=RESON_TRN_MBTRNDIR;
mbhbt["win.reson"]=MBHBT_MAPPER1;
trnhbt["win.reson"]=TRNHBT_MAPPER1;
trnuhbt["win.reson"]=TRNUHBT_MAPPER1;

TRN_RESON_HOST["linux.mbari"]=MBARI_LINUXVM_IP;
TRN_HOST["linux.mbari"]=MBARI_LINUXVM_IP;
TRN_LOGFILES["linux.mbari"]=LINUXVM_LOGFILES;
TRN_DATAFILES["linux.mbari"]=LINUXVM_DATAFILES;
TRN_MAPFILES["linux.mbari"]=LINUXVM_MAPFILES;
TRN_MBTRNDIR["linux.mbari"]=LINUXVM_TRN_MBTRNDIR;
mbhbt["linux.mbari"]=MBHBT_TEST;
trnhbt["linux.mbari"]=TRNHBT_TEST;
trnuhbt["linux.mbari"]=TRNUHBT_TEST;

TRN_RESON_HOST["linux.carson"]=CARSON_LINUXVM_IP;
TRN_HOST["linux.carson"]=CARSON_LINUXVM_IP;
TRN_LOGFILES["linux.carson"]=LINUXVM_LOGFILES;
TRN_DATAFILES["linux.carson"]=LINUXVM_DATAFILES;
TRN_MAPFILES["linux.carson"]=LINUXVM_MAPFILES;
TRN_MBTRNDIR["linux.carson"]=LINUXVM_TRN_MBTRNDIR;
mbhbt["linux.carson"]=MBHBT_TEST;
trnhbt["linux.carson"]=TRNHBT_TEST;
trnuhbt["linux.carson"]=TRNUHBT_TEST;

TRN_RESON_HOST["linux.home"]=HOME_LINUXVM_IP;
TRN_HOST["linux.home"]=HOME_LINUXVM_IP;
TRN_LOGFILES["linux.home"]=LINUXVM_LOGFILES;
TRN_DATAFILES["linux.home"]=LINUXVM_DATAFILES;
TRN_MAPFILES["linux.home"]=LINUXVM_MAPFILES;
TRN_MBTRNDIR["linux.home"]=LINUXVM_TRN_MBTRNDIR;
mbhbt["linux.home"]=MBHBT_TEST;
trnhbt["linux.home"]=TRNHBT_TEST;
trnuhbt["linux.home"]=TRNUHBT_TEST;

TRN_RESON_HOST["winvm.mbari"]=MBARI_WINVM_IP;
TRN_HOST["winvm.mbari"]=MBARI_WINVM_IP;
TRN_LOGFILES["winvm.mbari"]=WINVM_LOGFILES;
TRN_DATAFILES["winvm.mbari"]=WINVM_DATAFILES;
TRN_MAPFILES["winvm.mbari"]=WINVM_MAPFILES;
TRN_MBTRNDIR["winvm.mbari"]=WINVM_TRN_MBTRNDIR;
mbhbt["winvm.mbari"]=MBHBT_TEST;
trnhbt["winvm.mbari"]=TRNHBT_TEST;
trnuhbt["winvm.mbari"]=TRNUHBT_TEST;

TRN_RESON_HOST["winvm.carson"]=CARSON_WINVM_IP;
TRN_HOST["winvm.carson"]=CARSON_WINVM_IP;
TRN_LOGFILES["winvm.carson"]=WINVM_LOGFILES;
TRN_DATAFILES["winvm.carson"]=WINVM_DATAFILES;
TRN_MAPFILES["winvm.carson"]=WINVM_MAPFILES;
TRN_MBTRNDIR["winvm.carson"]=WINVM_TRN_MBTRNDIR;
mbhbt["winvm.carson"]=MBHBT_TEST;
trnhbt["winvm.carson"]=TRNHBT_TEST;
trnuhbt["winvm.carson"]=TRNUHBT_TEST;

TRN_RESON_HOST["winvm.home"]=HOME_WINVM_IP;
TRN_HOST["winvm.home"]=HOME_WINVM_IP;
TRN_LOGFILES["winvm.home"]=WINVM_LOGFILES;
TRN_DATAFILES["winvm.home"]=WINVM_DATAFILES;
TRN_MAPFILES["winvm.home"]=WINVM_MAPFILES;
TRN_MBTRNDIR["winvm.home"]=WINVM_TRN_MBTRNDIR;
mbhbt["winvm.home"]=MBHBT_TEST;
trnhbt["winvm.home"]=TRNHBT_TEST;
trnuhbt["winvm.home"]=TRNUHBT_TEST;

TRN_RESON_HOST["custom"]="";
TRN_HOST["custom"]="";
TRN_LOGFILES["custom"]="";
TRN_DATAFILES["custom"]="";
TRN_MAPFILES["custom"]="";
TRN_MBTRNDIR["custom"]="";
mbhbt["custom"]="";
trnhbt["custom"]="";
trnuhbt["custom"]="";

// help strings
help_topics["TRN_MBTRNDIR"]="mbtrnpp binary directory";
help_topics["TRN_TRN_RESON_HOST"]="7K center or emulator IP address";
help_topics["TRN_HOST"]="TRN output host IP (for TRN, TRNU, MB1 servers, etc.";
help_topics["TRN_DATAFILES"]="TRN_DATAFILES environment var\nSets TRN configuration output directory";
help_topics["TRN_MAPFILES"]="TRN_LOGFILES environment var\nSets TRN map file directory";
help_topics["TRN_LOGFILES"]="TRN_LOGFILES environment var\nSets TRN log output directory";
help_topics["verbose"]="mbtrnpp output level\n\
  0 : minmal output\n\
 <0 : more mbtrnpp debug (<-2 very verbose)\n\
 >0 : more mbsystem debug (>=-2 typical)";
help_topics["input"]="7K input source IP\n\n\
  use: socket:<address>:<port>:0";
help_topics["log-directory"]="MB-System, TRN log directory";
help_topics["swath-width"]="MB-System swath (deg)";
help_topics["soundings"]="MB1 bathymetry soundings (integer)";
help_topics["format"]="MB-System record format [MB1:88]";
help_topics["median-filter"]="Median filter settings\n\n\
  filter_threshold/n_along/n_across";
help_topics["output"]="MB-System output\n\
file:<file_name>\n\n\
[filename may include SESSION for current yyyymmdd-hhmmss]\n\n\
socket:<host>:<port> [MB1 server]\n\
socket               [MB1 server defaults]\n\
[socket same as --mb-out]";
help_topics["statsec"]="mbtrnpp profiling statistics update period (s)";
help_topics["statflags"]="mbtrnpp profiling flags\n\
 MSF_STATUS - include status\n\
 MSF_EVENT - include events\n\
 MSF_ASTAT - include aggregated stats\n\
 MSF_PSTAT - include periodic stats\n\
 MSF_READER - include R7K reader stats";
help_topics["delay"]="mbtrnpp loop delay (ms)";
help_topics["mbhbt"]="mb1 server heartbeat timeout (s)\n\n\
Drop client connections after timeout\n\
Value <=0 disables heartbeat; abandoned sockets persist";
help_topics["trnhbt"]="trn server heartbeat timeout (s)\n\n\
Drop client connections after timeout\n\
Value <=0 disables heartbeat; abandoned sockets persist";
help_topics["trnuhbt"]="trnu (TRN update pub/sub) server heartbeat timeout (s)\n\n\
Drop client connections after timeout\n\
Value <=0 disables heartbeat; abandoned sockets persist";
help_topics["trn-utm"]="TRN UTM zone (10:Monterey Bay)";
help_topics["mb-out"]="TRN MB1 output configuration\n\n\
Options for MB1 record output are selected using one or more comma separated values:\n\n\
mb1svr:<host>:<port> [enable MB1 pub/sub server]\n\
mb1                  [enable MB1 record loggine]\n\
reson                [enable S7K frame logging]\n\
file:<path>          [write mb1 records to file]\n\
file                 [write .mb1 file (see --output)]\n\
nomb1                [disable MB1 record logging]\n\
noreson              [disable S7K frame logging]\n\
nomb1svr             [disable MB1 pub/sub server]\n\
nombtrnpp            [disable MB1 message log (not recommended)]";
help_topics["trn-map"]="path to TRN map file or directory (for tiled maps)";
help_topics["trn-par"]="path to TRN particles file (required by TRN server but unused)";
help_topics["trn-cfg"]="path to TRN server config file";
help_topics["trn-mid"]="trn config mission ID (used for log directory prefix, e.g. <prefix>-TRN)\n[mbtrnpp TRN_LOGFILES environment variable must be set]";
help_topics["trn-decn"]="TRN update decimation modulus\n\n i.e., TRN is updated every n MB1 samples";
help_topics["trn-out"]="TRN output configuration\n\n\
Options for TRN output are configured using one or more comma separated values:\n\n\
trnsvr:<addr>:<port>  [enable TRN server (trn_server API)]\n\
trnusvr:<addr>:<port> [enable TRN update server (pub/sub)]\n\
trnu                  [enable TRN update logging]\n\
sout                  [enable TRN output to stdout]\n\
debug                 [enable TRN module debug output]";
help_topics["trn-en"]="TRN processing enable/disable\n\
[does not effect MB1 processing]"
help_topics["trn-mtype"]="TRN map type\n\
 1: TRN_MAP_DEM\n\
 2: TRN_MAP_BO";
help_topics["trn-ftype"]="TRN filter type\n\
 0: TRN_FILT_PARTICLE\n\
 1: TRN_FILT_NONE     \n\
 2: TRN_FILT_POINTMASS\n\
 3: TRN_FILT_PARTICLE\n\
 4: TRN_FILT_BANK";
help_topics["trn-fgrade"]="TRN filter grade\n\
 0: low grade\n\
 1: high grade";
help_topics["trn-reinit"]="Enable TRN filter reinits"
help_topics["trn-mweight"]="TRN modified weighting scheme\n\
 0 - No weighting modifications\n\
 1 - Shandor's original alpha modificatio\n\
 2 - Crossbeam with Shandor's weightin\n\
 3 - Subcloud with Shandor's original\n\
 4 - Subcloud with modified NIS always on";
help_topics["trn-ncov"]="TRN convergence criteria\n\
Northing Covariance limit"
help_topics["trn-nerr"]="TRN convergence criteria\n\
Northing Error limit"
help_topics["trn-ecov"]="TRN convergence criteria\n\
Easting Covariance limit"
help_topics["trn-eerr"]="TRN convergence criteria\n\
Easting Error limit"
help_topics["general"]="\n\
Values:\n\
 Empty values are removed from command line (use compiled default)\n\n\
Buttons :\n\
 Update : update command line output\n\
 Copy   : copy to clipboard \n\
 Reset  : reload defaults\n\n\
Placeholders : \n\
 Substitute selected parameter values:\n\n\
  TRN_RESON_HOST: use with [input]\n\
    7k Center host or emulator address, port \n\n\
  TRN_HOST  : use with [mb-out,trn-out]\n\
    mbtrnpp connection address,port\n\n\
  SESSION   : use with [output]\n\
    session ID (current date/time YYYYMMDD-HHMMSS)\n\n\
  TRN_DATAFILES : use with [trn-par,trn-cfg]\n\
    TRN config file directory\n\n\
  TRN_MAPFILES : use with [trn-map]\n\
    TRN map file directory\n\n\
  TRN_LOGFILES : use with [log-directory]\n\
    TRN log file directory\n";
help_topics["reson-host"]="Select reson host preset";
help_topics["trn-host"]="Select mbtrn host preset\n\n[affects directories, timeouts, etc]";
help_topics["set-trnlogfiles"]="Set TRN_LOGFILES environment on command line";

help_topics["copy"]="Copy command line to clipboard\n\n[calls update]";
help_topics["update"]="Update command line using current settings";
help_topics["load"]="Load preset values using Reson Host and MBTRN Host selections";
help_topics["export"]="Open config file in new tab";
help_topics["reset"]="";
help_topics["reinit-gain"]="Enable/disable gating TRN resets using sonar transmit gain";
help_topics["reinit-file"]="Reinitialize TRN every time a new file is read when parsing a datalist";
help_topics["reinit-xyoffset"]="Reinitialize TRN whenever the magnitude of the lateral converged offset exceeds specified limit";
help_topics["reinit-zoffset"]="Reinitialize TRN whenever the converged z-offset is outside specified range";
help_topics["covariance-magnitude-max"]="Convergence criteria: max covariance magnitude";
help_topics["convergence-repeat-min"]="Convergence criteria: min convergence repeat";
help_topics["tide-model"]="OTPS tide model file";

// show help string
function showhelp(key){
    var helpText = document.getElementById("helptext");
    // clear or set topic text
    if(key=='reset' || key=='general'){
        helpText.value="";
    }else{
        helpText.value=key+" :\n\n";
    }

    // set help output
    helpText.value+=help_topics[key];
    return;
}

// generate session id string (UTC YYYYMMDD-hhmmss)
function session_str(){
    var date = new Date();
    var Y = date.getUTCFullYear();
    var M = date.getUTCMonth()+1;
    M=(M<10?'0':'')+M;
    var D = (date.getUTCDate()<10?'0':'')+date.getUTCDate();
    var h = (date.getUTCHours()<10?'0':'')+date.getUTCHours();
    var m = (date.getUTCMinutes()<10?'0':'')+date.getUTCMinutes();
    var s = (date.getUTCSeconds()<10?'0':'')+date.getUTCSeconds();
    var str =Y+""+M+""+D+"-"+h+""+m+""+s;

    return str;
}

// generate TRN log session (YYYY.DDD)
function trn_session(){
    var date = new Date();
    var Y = date.getUTCFullYear();
    var M = date.getUTCMonth()+1;
    var D = date.getUTCDate();

    var leap = ( ((Y % 4 == 0 && Y % 100 != 0) || (Y % 400 == 0)) ? 1 : 0);
    var days=[
        [0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334],
        [0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335]
              ];

    var yday = days[leap][M] + D;
	// add leading zeros 
    var leadz = (yday<100 ? "0" : "")
    leadz += (yday<10?"0":"")

    var trn_session=Y+"."+leadz+yday;

    return trn_session;
}

// load preset configuration
function load_ctx(){

    var x = document.getElementById("cfgform");
    var rkey=x.elements["ctx-rhost"].value;
    var key=x.elements["ctx-mhost"].value;

    // initialize preset-specific/placeholder values
    x.elements["reson-host"].value=TRN_RESON_HOST[rkey];
    x.elements["out-host"].value=TRN_HOST[key];
    x.elements["mbtrndir"].value=TRN_MBTRNDIR[key];
    x.elements["trn_datafiles"].value=TRN_DATAFILES[key];
    x.elements["trn_mapfiles"].value=TRN_MAPFILES[key];
    x.elements["trn_logfiles"].value=TRN_LOGFILES[key];
    x.elements["mbhbt"].value=mbhbt[key];
    x.elements["trnhbt"].value=trnhbt[key];
    x.elements["trnuhbt"].value=trnuhbt[key];

    // initialize common defaults
    // (may reference placeholder values)
    x.elements["verbose"].value="-2";
    x.elements["input"].value="socket:TRN_RESON_HOST:7000:0";
    x.elements["log-directory"].value="TRN_LOGFILES";
    x.elements["swath-width"].value="90";
    x.elements["soundings"].value="11";
    x.elements["format"].value="88";
    x.elements["median-filter"].value="0.10/9/3";
    x.elements["output"].value="file:mbtrnpp_SESSION.mb1";
    x.elements["statsec"].value="30";
    x.elements["statflags"].value="MSF_STATUS|MSF_EVENT|MSF_ASTAT|MSF_PSTAT";
    x.elements["delay"].value="0";
    x.elements["trn-utm"].value="10";
    x.elements["mb-out"].value="mb1svr:TRN_HOST:27000";
    x.elements["trn-map"].value="TRN_MAPFILES/PortTiles";
    x.elements["trn-par"].value="TRN_DATAFILES/particles.cfg";
    x.elements["trn-cfg"].value="TRN_DATAFILES/mappingAUV_specs.cfg";
    x.elements["trn-mid"].value="mb-TRN_SESSION";
    x.elements["trn-decn"].value="9";
    x.elements["trn-out"].value="trnsvr:TRN_HOST:28000,trnu,trnusvr:TRN_HOST:8000";
    x.elements["trn-en"].value="en";
    x.elements["trn-mtype"].value="1";
    x.elements["trn-ftype"].value="2";
    x.elements["trn-fgrade"].value="1";
    x.elements["trn-reinit"].value="1";
    x.elements["trn-mweight"].value="4";
    x.elements["trn-ncov"].value="49";
    x.elements["trn-nerr"].value="200";
    x.elements["trn-ecov"].value="49";
    x.elements["trn-eerr"].value="200";
    x.elements["reinit-gain"].value="en";
    x.elements["reinit-file"].value="1";
    x.elements["reinit-xyoffset"].value="150.0";
    x.elements["reinit-zoffset"].value="2.0/2.0";
    x.elements["set-trnlogfiles"].value="en";
    x.elements["covariance-magnitude-max"].value="5.0";
    x.elements["convergence-repeat-min"].value="200";
    x.elements["tide-model"].value="";

    // clear help text
    showhelp('reset');
    // update output string
    update();
    return;
}

// initialize preset configuration values
function init_preset(key){

    var x = document.getElementById("cfgform");

    // initialize preset-specific/placeholder values
    x.elements["mbtrndir"].value=TRN_MBTRNDIR[key];
    x.elements["reson-host"].value=TRN_RESON_HOST[key];
    x.elements["out-host"].value=TRN_HOST[key];
    x.elements["trn_datafiles"].value=TRN_DATAFILES[key];
    x.elements["trn_mapfiles"].value=TRN_MAPFILES[key];
    x.elements["trn_logfiles"].value=TRN_LOGFILES[key];
    x.elements["mbhbt"].value=mbhbt[key];
    x.elements["trnhbt"].value=trnhbt[key];
    x.elements["trnuhbt"].value=trnuhbt[key];

    // initialize common defaults
    // (may reference placeholder values)
    x.elements["verbose"].value="-2";
    x.elements["input"].value="socket:TRN_RESON_HOST:7000:0";
    x.elements["log-directory"].value="TRN_LOGFILES";
    x.elements["swath-width"].value="90";
    x.elements["soundings"].value="11";
    x.elements["format"].value="88";
    x.elements["median-filter"].value="0.10/9/3";
    x.elements["output"].value="file:mbtrnpp_SESSION.mb1";
    x.elements["statsec"].value="30";
    x.elements["statflags"].value="MSF_STATUS|MSF_EVENT|MSF_ASTAT|MSF_PSTAT";
    x.elements["delay"].value="0";
    x.elements["trn-utm"].value="10";
    x.elements["mb-out"].value="mb1svr:TRN_HOST:27000";
    x.elements["trn-map"].value="TRN_MAPFILES/PortTiles";
    x.elements["trn-par"].value="TRN_DATAFILES/particles.cfg";
    x.elements["trn-cfg"].value="TRN_DATAFILES/mappingAUV_specs.cfg";
    x.elements["trn-mid"].value="mb-TRN_SESSION";
    x.elements["trn-decn"].value="9";
    x.elements["trn-out"].value="trnsvr:TRN_HOST:28000,trnu,trnusvr:TRN_HOST:8000";
    x.elements["trn-en"].value="en";
    x.elements["trn-mtype"].value="1";
    x.elements["trn-ftype"].value="2";
    x.elements["trn-fgrade"].value="1";
    x.elements["trn-reinit"].value="1";
    x.elements["trn-mweight"].value="4";
    x.elements["trn-ncov"].value="49";
    x.elements["trn-nerr"].value="200";
    x.elements["trn-ecov"].value="49";
    x.elements["trn-eerr"].value="200";
    x.elements["reinit-gain"].value="en";
    x.elements["reinit-file"].value="1";
    x.elements["reinit-xyoffset"].value="150.0";
    x.elements["reinit-zoffset"].value="2.0/2.0";
    x.elements["set-trnlogfiles"].value="en";
    x.elements["covariance-magnitude-max"].value="5.0";
    x.elements["convergence-repeat-min"].value="200";
    x.elements["tide-model"].value="";

    // clear help text
    showhelp('reset');
    // update output string
    update();
    return;
}

// substitute current values for placeholders
// (zero or more instances)
// returns empty string if option value blank
function sub_placeholder(form,optkey,pstr,pval){

    var retval="";
    var optstr='--'+optkey+'=';
    if(form.elements[optkey].value.length>0){
        var tmp=form.elements[optkey].value;
        // iterate until no placeholder instances
        while(tmp.indexOf(pstr)>=0){
            // replace next placeholder
            b=tmp.indexOf(pstr);
            var pre=tmp.substr(0,b);
            var post=tmp.substr(b+pstr.length);
            tmp = pre+pval+post+" "
        }
        retval = optstr+tmp;
    }
    return retval;
}

// update command line output text
function update(){

    var x = document.getElementById("cfgform");

    // update environment values with user inputs
    TRN_RESON_HOST["current"]=x.elements["reson-host"].value;
    TRN_HOST["current"]=x.elements["out-host"].value;
    TRN_MBTRNDIR["current"]=x.elements["mbtrndir"].value;
    TRN_DATAFILES["current"]=x.elements["trn_datafiles"].value;
    TRN_MAPFILES["current"]=x.elements["trn_mapfiles"].value;
    TRN_LOGFILES["current"]=x.elements["trn_logfiles"].value;

    // build command line
    var text = TRN_MBTRNDIR["current"]+"/mbtrnpp ";
    if(x.elements["verbose"].value.length>0)
        text += '--verbose='+x.elements["verbose"].value+" ";
    if(x.elements["swath-width"].value.length>0)
        text += '--swath-width='+x.elements["swath-width"].value+" ";
    if(x.elements["soundings"].value.length>0)
        text += '--soundings='+x.elements["soundings"].value+" ";
    if(x.elements["format"].value.length>0)
        text += '--format='+x.elements["format"].value+" ";
    if(x.elements["median-filter"].value.length>0)
        text += '--median-filter='+x.elements["median-filter"].value+" ";
    if(x.elements["statsec"].value.length>0)
        text += '--statsec='+x.elements["statsec"].value+" ";
    if(x.elements["statflags"].value.length>0)
        text += '--statflags='+x.elements["statflags"].value+" ";
    if(x.elements["delay"].value.length>0 && x.elements["delay"].value>0)
        text += '--delay='+x.elements["delay"].value+" ";
    if(x.elements["mbhbt"].value.length>0)
    	text += '--mbhbt='+x.elements["mbhbt"].value+" ";
    if(x.elements["trnhbt"].value.length>0)
    	text += '--trnhbt='+x.elements["trnhbt"].value+" ";
    if(x.elements["trnuhbt"].value.length>0)
    	text += '--trnuhbt='+x.elements["trnuhbt"].value+" ";
    if(x.elements["trn-utm"].value.length>0)
    	text += '--trn-utm='+x.elements["trn-utm"].value+" ";
    if(x.elements["trn-decn"].value.length>0)
    	text += '--trn-decn='+x.elements["trn-decn"].value+" "
    if(x.elements["trn-en"].value.length>0)
    	text += '--trn-'+x.elements["trn-en"].value+" ";
    if(x.elements["trn-mtype"].value.length>0)
        text += '--trn-mtype='+x.elements["trn-mtype"].value+" ";
    if(x.elements["trn-ftype"].value.length>0)
        text += '--trn-ftype='+x.elements["trn-ftype"].value+" ";
    if(x.elements["trn-fgrade"].value.length>0)
        text += '--trn-fgrade='+x.elements["trn-fgrade"].value+" ";
    if(x.elements["trn-reinit"].value.length>0)
        text += '--trn-reinit='+x.elements["trn-reinit"].value+" ";
    if(x.elements["trn-mweight"].value.length>0)
        text += '--trn-mweight='+x.elements["trn-mweight"].value+" ";
    if(x.elements["trn-ncov"].value.length>0)
        text += '--trn-ncov='+x.elements["trn-ncov"].value+" ";
    if(x.elements["trn-nerr"].value.length>0)
        text += '--trn-nerr='+x.elements["trn-nerr"].value+" ";
    if(x.elements["trn-ecov"].value.length>0)
        text += '--trn-ecov='+x.elements["trn-ecov"].value+" ";
    if(x.elements["trn-eerr"].value.length>0)
        text += '--trn-eerr='+x.elements["trn-eerr"].value+" ";
    if(x.elements["reinit-gain"].value=='en')
        text += '--reinit-gain'+" ";
    if(x.elements["reinit-file"].value.length>0)
        text += '--reinit-file='+x.elements["reinit-file"].value+" ";
    if(x.elements["reinit-xyoffset"].value.length>0)
        text += '--reinit-xyoffset='+x.elements["reinit-xyoffset"].value+" ";
    if(x.elements["reinit-zoffset"].value.length>0)
        text += '--reinit-zoffset='+x.elements["reinit-zoffset"].value+" ";
    if(x.elements["set-trnlogfiles"].value=='en')
        text = "TRN_LOGFILES=\""+TRN_LOGFILES["current"]+"\" "+text;
    if(x.elements["reinit-zoffset"].value.length>0)
        text += '--reinit-zoffset='+x.elements["reinit-zoffset"].value+" ";
    if(x.elements["covariance-magnitude-max"].value.length>0)
        text += '--covariance-magnitude-max='+x.elements["covariance-magnitude-max"].value+" ";
    if(x.elements["convergence-repeat-min"].value.length>0)
        text += '--convergence-repeat-min='+x.elements["convergence-repeat-min"].value+" ";
    if(x.elements["tide-model"].value.length>0)
        text += '--tide-model='+x.elements["tide-model"].value+" ";

    // these parameters support substitutions
    text += sub_placeholder(x,"input","TRN_RESON_HOST",TRN_RESON_HOST["current"]);
    text += sub_placeholder(x,"log-directory","TRN_LOGFILES",TRN_LOGFILES["current"]);
    text += sub_placeholder(x,"trn-map","TRN_MAPFILES",TRN_MAPFILES["current"]);
    text += sub_placeholder(x,"trn-par","TRN_DATAFILES",TRN_DATAFILES["current"]);
    text += sub_placeholder(x,"trn-cfg","TRN_DATAFILES",TRN_DATAFILES["current"]);
    text += sub_placeholder(x,"output","SESSION",session_str());
    text += sub_placeholder(x,"mb-out","TRN_HOST",TRN_HOST["current"]);
    text += sub_placeholder(x,"trn-out","TRN_HOST",TRN_HOST["current"]);
    text += sub_placeholder(x,"trn-mid","TRN_SESSION",trn_session());

    // set the output value
    var ofrm = document.getElementById("outtext");
    ofrm.value=text;
    return;
}

// copy command line to clipboard
function copyclip(){
    update();
    // Get the text field
    var copyText = document.getElementById("outtext");

    // Select the text field
    copyText.select();
    // for mobile devices...
    copyText.setSelectionRange(0, 99999);

    // Copy the text inside the text field
    document.execCommand("copy");
    document.getElementById("cbutton").focus();
    return;
}

function cfg2str(){
    var x = document.getElementById("cfgform");
    var y = document.getElementById("outform");
    var verbose=true;
    if(y.elements["verbose-export"].value=='dis')
        verbose=false;
    var retval="";

    if(verbose){
        retval+="#  mbtrnpp configuration\n";
        retval+="#  [generated by mbtrncfg]\n";
        retval+="#\n";
        retval+="# - option naming same as command line (w/o '--')\n";
        retval+="# - use '#' or '//' for comments\n";
        retval+="# - \"opt\" annotations indicate option name and internal type\n";
        retval+="    # - some options support variable substitution:\n";
        retval+="    #     option        variable          description\n";
        retval+="    #     ------        --------          -----------\n";
        retval+="    #     output        SESSION           mbtrpp session ID (YYYYMMDD-hhmmss)\n";
        retval+="    #     input         TRN_RESON_HOST[1] input socket IP address\n";
        retval+="    #     mb-out        TRN_HOST[1]       TRN processing host IP\n";
        retval+="    #     trn-out       TRN_HOST\n";
        retval+="    #     trn-mid       TRN_SESSION       TRN mission ID (YYYY-JJJ) (JJJ: year day)\n";
        retval+="    #     log-directory TRN_LOGFILES[2]   TRN log directory\n";
        retval+="    #     trn-map       TRN_MAPFILES[2]   TRN map file directory\n";
        retval+="    #     trn-par       TRN_DATAFILES[2]  TRN data/cfg file directory\n";
        retval+="    #     trn-cfg       TRN_DATAFILES\n";
        retval+="    #   Notes\n";
        retval+="    #     [1] use current host IP address if environment unset\n";
        retval+="    #     [2] use current directory if environment unset\n";
    }

    retval+="\n";
    if(verbose){
        retval+="// opt verbose [int]\n";
        retval+="// debug output level\n";
        retval+="// <0 : TRN debug (-2 typical)\n";
        retval+="// >0 : MB-System debug (>2 very verbose)\n";
    }
    retval+="verbose="+x.elements["verbose"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt log-directory [mb_path]\n";
        retval+="// log directory\n";
    }
    retval+="log-directory="+x.elements["log-directory"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt input [input_mode_t]\n";
        retval+="// input data source specifier\n";
        retval+="// file_path\n";
        retval+="// socket:IP:port:bcast_group\n";
    }
    retval+="input="+x.elements["input"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt output [?]\n";
        retval+="// output mode specifier\n";
    }
    retval+="output="+x.elements["output"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt swath-width [double]\n";
        retval+="// sonar swath (decimal degrees)\n";
    }
    retval+="swath-width="+x.elements["swath-width"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt soundings [int]\n";
        retval+="// number of sonar soundings to output\n";
    }
    retval+="soundings="+x.elements["soundings"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt median-filter [?]\n";
        retval+="// median filter parameters\n";
        retval+="// <threshold>/<n_across>/<n_along>\n";
    }
    retval+="median-filter="+x.elements["median-filter"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt format [int]\n";
        retval+="// input data format\n";
    }
    retval+="format="+x.elements["format"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt platform-file [mb_path]\n";
        retval+="// description TBD\n";
    }
    retval+="#platform-file=\n";

    retval+="\n";
    if(verbose){
        retval+="// opt platform-target-sensor [int]\n";
        retval+="// description TBD\n";
    }
    retval+="#platform-target-sensor=\n";

    retval+="\n";
    if(verbose){
        retval+="// opt projection\n";
        retval+="// cartographic(?) projection ID\n";
    }
    retval+="#projection=\n";

    retval+="\n";
    if(verbose){
        retval+="// opt mbhbn [int]\n";
        retval+="// MB1 server heartbeat modulus\n";
        retval+="// (timeout preferred, use mbhbt)\n";
    }
    retval+="#mbhbn\n";

    retval+="\n";
    if(verbose){
        retval+="// opt mbhbt [double]\n";
        retval+="// MB1 server heartbeat timeout (s)\n";
    }
    retval+="mbhbt="+x.elements["mbhbt"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trnhbt [double]\n";
        retval+="// TRN server heartbeat timeout (s)\n";
    }
    retval+="trnhbt="+x.elements["trnhbt"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trnuhbt [double]\n";
        retval+="// TRNU (udp update) server heartbeat timeout (s)\n";
    }
    retval+="trnuhbt="+x.elements["trnuhbt"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt delay [int64_t]\n";
        retval+="// Delay main TRN processing loop (msec)\n";
    }
    retval+="delay="+x.elements["delay"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt statsec [double]\n";
        retval+="// TRN profiling logging interval (s)\n";
    }
    retval+="statsec="+x.elements["statsec"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt statflags [mstats_flags]\n";
        retval+="// TRN profiling flags\n";
        retval+="// MSF_STATUS - include status\n";
        retval+="// MSF_EVENT - include events\n";
        retval+="// MSF_ASTAT - include aggregated stats\n";
        retval+="// MSF_PSTAT - include periodic stats\n";
        retval+="// MSF_READER - include R7K reader stats\n";
    }
    retval+="statflags="+x.elements["statflags"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-en [bool]\n";
        retval+="// opt trn-dis [bool]\n";
        retval+="// enable/disable TRN processing\n";
        retval+="// use Y/1: enable N/0: disable\n";
    }
    retval+="trn-en="+x.elements["trn-en"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-utm [long]\n";
        retval+="// UTM zone for TRN processing (1-60)\n";
        retval+="// Monterey Bay : 10\n";
        retval+="//        Axial :  9\n";
    }
    retval+="trn-utm="+x.elements["trn-utm"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-map [char*]\n";
        retval+="// TRN map file (required for TRN processing)\n";
        retval+="// (may be a directory path for tiled map)\n";
    }
    retval+="trn-map="+x.elements["trn-map"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-cfg [char*]\n";
        retval+="// TRN configuration file (required for TRN processing)\n";
    }
    retval+="trn-cfg="+x.elements["trn-cfg"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-par [char*]\n";
        retval+="// TRN particles file (optional for TRN processing)\n";
    }
    retval+="trn-par="+x.elements["trn-par"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-mid [char*]\n";
        retval+="// TRN mission ID\n";
        retval+="// (used for TRN log directory name prefix)\n";
    }
    retval+="trn-mid="+x.elements["trn-mid"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-mtype [int]\n";
        retval+="// TRN map type\n";
        retval+="// TRN_MAP_DEM  1\n";
        retval+="// TRN_MAP_BO   2\n";
    }
    retval+="trn-mtype="+x.elements["trn-mtype"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-ftype [int]\n";
        retval+="// TRN filter type\n";
        retval+="// TRN_FILT_NONE       0\n";
        retval+="// TRN_FILT_POINTMASS  1\n";
        retval+="// TRN_FILT_PARTICLE   2\n";
        retval+="// TRN_FILT_BANK       3\n";
    }
    retval+="trn-ftype="+x.elements["trn-ftype"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-fgrade [int]\n";
        retval+="// filter grade\n";
        retval+="// 0: use low grade filter\n";
        retval+="// 1: use high grade filter\n";
    }
    retval+="trn-fgrade="+x.elements["trn-fgrade"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-reinit [int]\n";
        retval+="// enable filter reinit\n";
        retval+="// 0: don't allow filter reinit\n";
        retval+="// 1: allow reinit\n";
    }
    retval+="trn-reinit="+x.elements["trn-reinit"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-mweight [int]\n";
        retval+="// use modified weighting\n";
        retval+="// 0 - No weighting modifications\n";
        retval+="// 1 - Shandor's original alpha modification\n";
        retval+="// 2 - Crossbeam with Shandor's weighting\n";
        retval+="// 3 - Subcloud with Shandor's origina\n";
        retval+="// 4 - Subcloud with modified NIS always on\n";
    }
    retval+="trn-mweight="+x.elements["trn-mweight"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-ncov [double]\n";
        retval+="// TRN convergence criteria\n";
        retval+="// northing covariance limit\n";
    }
    retval+="trn-ncov="+x.elements["trn-ncov"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-nerr [double]\n";
        retval+="// TRN convergence criteria\n";
        retval+="// northing error limit\n";
        retval+="//trn-nerr=50\n";
    }
    retval+="trn-nerr="+x.elements["trn-nerr"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-ecov [double]\n";
        retval+="// TRN convergence criteria\n";
        retval+="// easting covariance limit\n";
    }
    retval+="trn-ecov="+x.elements["trn-ecov"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-eerr [double]\n";
        retval+="// TRN convergence criteria\n";
        retval+="// easting error limit\n";
    }
    retval+="trn-eerr="+x.elements["trn-eerr"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt mb-out [?]\n";
        retval+="// MB1 output specifier\n";
        retval+="// comma delimited list including one or more of:\n";
        retval+="// mb1svr:<ip>:<port>  - enable MB1 server\n";
        retval+="// mb1                 - enable MB1 binary record logging\n";
        retval+="// file:<path>         - enable MB1 binary record logging\n";
        retval+="// file                - enable MB1 binary record logging (use default name)\n";
        retval+="// reson               - enable reson S7K frame logging\n";
        retval+="// nomb1               - disable MB1 binary record logging\n";
        retval+="// noreson             - disable reson S7K frame logging\n";
        retval+="// nomb1               - disable MB1 server\n";
        retval+="// nombtrnpp           - disable mbtrnpp message log (not recommended)\n";
    }
    retval+="mb-out="+x.elements["mb-out"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-out [?]\n";
        retval+="// TRN output specifier\n";
        retval+="// comma delimited list including one or more of:\n";
        retval+="// trnsvr:<ip>:<port>  - enable trn server interface\n";
        retval+="// trnu                - enable TRN update ascii logging\n";
        retval+="// trnub               - enable TRN update binary logging\n";
        retval+="// trnusvr:<ip>:<port> - enable TRN update server\n";
        retval+="// sout                - enable TRN output to stdout\n";
        retval+="// serr                - enable TRN output to stderr\n";
        retval+="// debug               - enable TRN debug output\n";
        retval+="// notrnsvr            - disable trn server interface\n";
        retval+="// notrnusvr           - disable TRN update server\n";
    }
    retval+="trn-out="+x.elements["trn-out"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-decn [int]\n";
        retval+="// TRN updpate decimation modulus\n";
        retval+="// (update TRN every nth MB1 sample)\n";
    }
    retval+="trn-decn="+x.elements["trn-decn"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt trn-decs [double]\n";
        retval+="// TRN updpate decimation interval (decimal seconds)\n";
        retval+="// (update TRN every dec seconds)\n";
    }
    retval+="#trn-decs=3.0\n";

    retval+="\n";
    if(verbose){
        retval+="// opt reinit-gain [none]\n";
        retval+="// enable/disable gating TRN resets using sonar transmit gain\n";
    }
    if(x.elements["reinit-gain"].value=="en")
        retval+="reinit-gain\n";
    else
        retval+="#reinit-gain\n";

    retval+="\n";
    if(verbose){
        retval+="// opt reinit-file [bool]\n";
        retval+="// reinitialize TRN every time a new\n";
        retval+="// file is read when parsing a datalist\n";
    }
    retval+="reinit-file="+x.elements["reinit-file"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt reinit-xyoffset [double]\n";
        retval+="// reinitialize TRN whenever the magnitude of the\n";
        retval+="// lateral converged offset exceeds specified limit\n";
    }
    retval+="reinit-xyoffset="+x.elements["reinit-xyoffset"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt reinit-zoffset [double]\n";
        retval+="// reinitialize TRN whenever the converged z-offset\n";
        retval+="// is is outside specified range.\n";
    }
    retval+="reinit-zoffset="+x.elements["reinit-zoffset"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt covariance-magnitude-max [double]\n";
        retval+="// TRN convergence max covariance limit\n";
    }
    retval+="covariance-magnitude-max="+x.elements["covariance-magnitude-max"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt convergence-repeat-min [int]\n";
        retval+="// TRN convergence minimum repeat limit\n";
    }
    retval+="convergence-repeat-min="+x.elements["convergence-repeat-min"].value+"\n";

    retval+="\n";
    if(verbose){
        retval+="// opt tide-model [path]\n";
        retval+="// OTPS tide model file\n";
    }
    if(x.elements["tide-model"].value.length>0)
        retval+="tide-model="+x.elements["tide-model"].value+"\n";
    else
        retval+="#tide-model=\n";
    return retval;
}

function save_cfg(){

    var y = document.getElementById("saveform");
    var filename=y.elements["export-path"].value;
    var docstr=y.elements["doc-data"].value;

    var element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + docstr);
    element.setAttribute('download', filename);

    element.style.display = 'false';
    document.body.appendChild(element);
    element.click();

}


function write_cfg(){

    var docstr=cfg2str();
    var w = window.open();
    w.document.writeln("<!DOCTYPE html>");
    w.document.writeln("<html>");
    w.document.writeln("<head>");

    w.document.writeln("<title>mbtrnpp Config File</title>");

    w.document.writeln("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    w.document.writeln("<meta http-equiv=\"Content-Style-Type\" content=\"text/css\">");
    w.document.writeln("<meta http-equiv=\"Content-Language\" content=\"en\">");
    w.document.writeln("<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\">");
    w.document.writeln("<meta http-equiv=\"Pragma\" content=\"no-cache\">");
    w.document.writeln("<meta http-equiv=\"Expires\" content=\"0\">");

    w.document.writeln("<script type=\"text/javascript\" language=\"Javascript\" SRC=\"src/mbtrncfg.js\"></script>");

    w.document.writeln("<style type=\"text/css\">");
    w.document.writeln("<!--");
    w.document.writeln("@import \"css/mbtrncfg.css\";");
    w.document.writeln("-->");
    w.document.writeln("</style>");

    w.document.writeln("</head>");

    w.document.writeln("<body>");
    w.document.writeln("<div style=\"height:80px; width:100%; top:0px;position:fixed;background-color:inherit;\">");
    w.document.writeln("<form method=POST name=\"output\" id=\"saveform\" enctype=\"text/plain\" style=\"margin: 20px 10px 20px 10px;\">");
    w.document.writeln("<input type='hidden' name='doc-data' value=\""+encodeURIComponent(docstr)+"\">");
    w.document.writeln("<button type=\"button\" class=\"button buttony buttonsm\" onclick=\"javascript:save_cfg();\">Download</button>");
    w.document.writeln("<input type=\"text\" size=\"32\" name=\"export-path\" value='mbtrnpp-auto.cfg' >");
    w.document.writeln("</form>");
    w.document.writeln("</div>");

    w.document.writeln("<pre class=\"cfgtext\">");
    var escdoc = docstr.replace(/&/g,"&amp;").replace(/>/g,"&gt;").replace(/</g,"&lt;");
    w.document.writeln(escdoc);
    w.document.writeln("</pre>");


    w.document.writeln("</body>");
    w.document.writeln("</html>");

}

