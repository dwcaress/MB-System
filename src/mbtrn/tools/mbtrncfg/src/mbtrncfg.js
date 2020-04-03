// variable declarations, initialization
var help_topics=[];
var RESON_HOST=[];
var MBTRN_HOST=[];
var TRN_LOGFILES=[];
var TRN_DATAFILES=[];
var TRN_MAPFILES=[];
var MBTRNDIR=[];
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
var RESON_MBTRNDIR="/usr/local/bin"

var LINUXVM_LOGFILES="/home/headley/tmp/logs";
var LINUXVM_DATAFILES="/mnt/vmhost/config";
var LINUXVM_MAPFILES="/mnt/vmhost/maps";
var LINUXVM_MBTRNDIR="/mnt/vmhost/git/mbsys-trn/MB-System/src/mbtrnutils"

var WINVM_LOGFILES="/cygdrive/z/win_share/test/logs";
var WINVM_DATAFILES="/cygdrive/z/win_share/test/config";
var WINVM_MAPFILES="/cygdrive/z/win_share/test/maps";
var WINVM_MBTRNDIR="/home/headley/git/MB-System/src/mbtrnutils"

//initialize session
var SESSION=session_str();

// initialize presets
RESON_HOST["win.reson"]=MAPPER1_RESON_IP;
MBTRN_HOST["win.reson"]=MAPPER1_RESON_IP;
TRN_LOGFILES["win.reson"]=RESON_LOGFILES;
TRN_DATAFILES["win.reson"]=RESON_DATAFILES;
TRN_MAPFILES["win.reson"]=RESON_MAPFILES;
MBTRNDIR["win.reson"]=RESON_MBTRNDIR;
mbhbt["win.reson"]=MBHBT_MAPPER1;
trnhbt["win.reson"]=TRNHBT_MAPPER1;
trnuhbt["win.reson"]=TRNUHBT_MAPPER1;

RESON_HOST["linux.mbari"]=MBARI_LINUXVM_IP;
MBTRN_HOST["linux.mbari"]=MBARI_LINUXVM_IP;
TRN_LOGFILES["linux.mbari"]=LINUXVM_LOGFILES;
TRN_DATAFILES["linux.mbari"]=LINUXVM_DATAFILES;
TRN_MAPFILES["linux.mbari"]=LINUXVM_MAPFILES;
MBTRNDIR["linux.mbari"]=LINUXVM_MBTRNDIR;
mbhbt["linux.mbari"]=MBHBT_TEST;
trnhbt["linux.mbari"]=TRNHBT_TEST;
trnuhbt["linux.mbari"]=TRNUHBT_TEST;

RESON_HOST["linux.carson"]=CARSON_LINUXVM_IP;
MBTRN_HOST["linux.carson"]=CARSON_LINUXVM_IP;
TRN_LOGFILES["linux.carson"]=LINUXVM_LOGFILES;
TRN_DATAFILES["linux.carson"]=LINUXVM_DATAFILES;
TRN_MAPFILES["linux.carson"]=LINUXVM_MAPFILES;
MBTRNDIR["linux.carson"]=LINUXVM_MBTRNDIR;
mbhbt["linux.carson"]=MBHBT_TEST;
trnhbt["linux.carson"]=TRNHBT_TEST;
trnuhbt["linux.carson"]=TRNUHBT_TEST;

RESON_HOST["linux.home"]=HOME_LINUXVM_IP;
MBTRN_HOST["linux.home"]=HOME_LINUXVM_IP;
TRN_LOGFILES["linux.home"]=LINUXVM_LOGFILES;
TRN_DATAFILES["linux.home"]=LINUXVM_DATAFILES;
TRN_MAPFILES["linux.home"]=LINUXVM_MAPFILES;
MBTRNDIR["linux.home"]=LINUXVM_MBTRNDIR;
mbhbt["linux.home"]=MBHBT_TEST;
trnhbt["linux.home"]=TRNHBT_TEST;
trnuhbt["linux.home"]=TRNUHBT_TEST;

RESON_HOST["winvm.mbari"]=MBARI_WINVM_IP;
MBTRN_HOST["winvm.mbari"]=MBARI_WINVM_IP;
TRN_LOGFILES["winvm.mbari"]=WINVM_LOGFILES;
TRN_DATAFILES["winvm.mbari"]=WINVM_DATAFILES;
TRN_MAPFILES["winvm.mbari"]=WINVM_MAPFILES;
MBTRNDIR["winvm.mbari"]=WINVM_MBTRNDIR;
mbhbt["winvm.mbari"]=MBHBT_TEST;
trnhbt["winvm.mbari"]=TRNHBT_TEST;
trnuhbt["winvm.mbari"]=TRNUHBT_TEST;

RESON_HOST["winvm.carson"]=CARSON_WINVM_IP;
MBTRN_HOST["winvm.carson"]=CARSON_WINVM_IP;
TRN_LOGFILES["winvm.carson"]=WINVM_LOGFILES;
TRN_DATAFILES["winvm.carson"]=WINVM_DATAFILES;
TRN_MAPFILES["winvm.carson"]=WINVM_MAPFILES;
MBTRNDIR["winvm.carson"]=WINVM_MBTRNDIR;
mbhbt["winvm.carson"]=MBHBT_TEST;
trnhbt["winvm.carson"]=TRNHBT_TEST;
trnuhbt["winvm.carson"]=TRNUHBT_TEST;

RESON_HOST["winvm.home"]=HOME_WINVM_IP;
MBTRN_HOST["winvm.home"]=HOME_WINVM_IP;
TRN_LOGFILES["winvm.home"]=WINVM_LOGFILES;
TRN_DATAFILES["winvm.home"]=WINVM_DATAFILES;
TRN_MAPFILES["winvm.home"]=WINVM_MAPFILES;
MBTRNDIR["winvm.home"]=WINVM_MBTRNDIR;
mbhbt["winvm.home"]=MBHBT_TEST;
trnhbt["winvm.home"]=TRNHBT_TEST;
trnuhbt["winvm.home"]=TRNUHBT_TEST;

RESON_HOST["custom"]="";
MBTRN_HOST["custom"]="";
TRN_LOGFILES["custom"]="";
TRN_DATAFILES["custom"]="";
TRN_MAPFILES["custom"]="";
MBTRNDIR["custom"]="";
mbhbt["custom"]="";
trnhbt["custom"]="";
trnuhbt["custom"]="";

// help strings
help_topics["MBTRNDIR"]="mbtrnpp binary directory";
help_topics["RESON_HOST"]="7K center or emulator IP address";
help_topics["MBTRN_HOST"]="TRN output host IP (for TRN, TRNU, MB1 servers, etc.";
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
help_topics["swath"]="MB-System swath (deg)";
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
help_topics["statsec"]="mbtrnpp statistics update period (s)";
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
help_topics["trn-log"]="trn config log directory prefix (e.g. <prefix>-TRN)\n[mbtrnpp TRN_LOGFILES environment variable must be set]";
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
help_topics["general"]="\n\
Values:\n\
 Empty values are removed from command line (use compiled default)\n\n\
Buttons :\n\
 Update : update command line output\n\
 Copy   : copy to clipboard \n\
 Reset  : reload defaults\n\n\
Placeholders : \n\
 Substitute selected parameter values:\n\n\
  RESON_HOST: use with [input]\n\
    7k Center host or emulator address, port \n\n\
  MBTRN_HOST  : use with [mb-out,trn-out]\n\
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
help_topics["mbtrn-host"]="Select mbtrn host preset\n\n[affects directories, timeouts, etc]";
help_topics["set-trnlogfiles"]="Set TRN_LOGFILES environment on command line";

help_topics["copy"]="Copy command line to clipboard\n\n[calls update]";
help_topics["update"]="Update command line using current settings";
help_topics["load"]="Load preset values using Reson Host and MBTRN Host selections";
help_topics["reset"]="";

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
    x.elements["reson-host"].value=RESON_HOST[rkey];
    x.elements["out-host"].value=MBTRN_HOST[key];
    x.elements["mbtrndir"].value=MBTRNDIR[key];
    x.elements["trn_datafiles"].value=TRN_DATAFILES[key];
    x.elements["trn_mapfiles"].value=TRN_MAPFILES[key];
    x.elements["trn_logfiles"].value=TRN_LOGFILES[key];
    x.elements["mbhbt"].value=mbhbt[key];
    x.elements["trnhbt"].value=trnhbt[key];
    x.elements["trnuhbt"].value=trnuhbt[key];

    // initialize common defaults
    // (may reference placeholder values)
    x.elements["verbose"].value="-2";
    x.elements["input"].value="socket:RESON_HOST:7000:0";
    x.elements["log-directory"].value="TRN_LOGFILES";
    x.elements["swath"].value="90";
    x.elements["soundings"].value="11";
    x.elements["format"].value="88";
    x.elements["median"].value="0.10/9/3";
    x.elements["output"].value="file:mbtrnpp_SESSION.mb1";
    x.elements["statsec"].value="30";
    x.elements["trn-utm"].value="10";
    x.elements["mb-out"].value="mb1svr:MBTRN_HOST:27000";
    x.elements["trn-map"].value="TRN_MAPFILES/PortTiles";
    x.elements["trn-par"].value="TRN_DATAFILES/particles.cfg";
    x.elements["trn-cfg"].value="TRN_DATAFILES/mappingAUV_specs.cfg";
    x.elements["trn-log"].value="mb-TRN_SESSION";
    x.elements["trn-decn"].value="9";
    x.elements["trn-out"].value="trnsvr:MBTRN_HOST:28000,trnu,trnusvr:MBTRN_HOST:8000";
    x.elements["trn-en"].value="en";
    x.elements["set-trnlogfiles"].value="en";

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
    x.elements["mbtrndir"].value=MBTRNDIR[key];
    x.elements["reson-host"].value=RESON_HOST[key];
    x.elements["out-host"].value=MBTRN_HOST[key];
    x.elements["trn_datafiles"].value=TRN_DATAFILES[key];
    x.elements["trn_mapfiles"].value=TRN_MAPFILES[key];
    x.elements["trn_logfiles"].value=TRN_LOGFILES[key];
    x.elements["mbhbt"].value=mbhbt[key];
    x.elements["trnhbt"].value=trnhbt[key];
    x.elements["trnuhbt"].value=trnuhbt[key];

    // initialize common defaults
    // (may reference placeholder values)
    x.elements["verbose"].value="-2";
    x.elements["input"].value="socket:RESON_HOST:7000:0";
    x.elements["log-directory"].value="TRN_LOGFILES";
    x.elements["swath"].value="90";
    x.elements["soundings"].value="11";
    x.elements["format"].value="88";
    x.elements["median"].value="0.10/9/3";
    x.elements["output"].value="file:mbtrnpp_SESSION.mb1";
    x.elements["statsec"].value="30";
    x.elements["trn-utm"].value="10";
    x.elements["mb-out"].value="mb1svr:MBTRN_HOST:27000";
    x.elements["trn-map"].value="TRN_MAPFILES/PortTiles";
    x.elements["trn-par"].value="TRN_DATAFILES/particles.cfg";
    x.elements["trn-cfg"].value="TRN_DATAFILES/mappingAUV_specs.cfg";
    x.elements["trn-log"].value="mb-TRN_SESSION";
    x.elements["trn-decn"].value="9";
    x.elements["trn-out"].value="trnsvr:MBTRN_HOST:28000,trnu,trnusvr:MBTRN_HOST:8000";
    x.elements["trn-en"].value="en";
    x.elements["set-trnlogfiles"].value="en";

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
    RESON_HOST["current"]=x.elements["reson-host"].value;
    MBTRN_HOST["current"]=x.elements["out-host"].value;
    MBTRNDIR["current"]=x.elements["mbtrndir"].value;
    TRN_DATAFILES["current"]=x.elements["trn_datafiles"].value;
    TRN_MAPFILES["current"]=x.elements["trn_mapfiles"].value;
    TRN_LOGFILES["current"]=x.elements["trn_logfiles"].value;

    // build command line
    var text = MBTRNDIR["current"]+"/mbtrnpp ";
    if(x.elements["verbose"].value.length>0)
        text += '--verbose='+x.elements["verbose"].value+" ";
    if(x.elements["swath"].value.length>0)
        text += '--swath='+x.elements["swath"].value+" ";
    if(x.elements["soundings"].value.length>0)
        text += '--soundings='+x.elements["soundings"].value+" ";
    if(x.elements["format"].value.length>0)
        text += '--format='+x.elements["format"].value+" ";
    if(x.elements["median"].value.length>0)
        text += '--median-filter='+x.elements["median"].value+" ";
    if(x.elements["statsec"].value.length>0)
    	text += '--statsec='+x.elements["statsec"].value+" ";
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
    if(x.elements["set-trnlogfiles"].value=='en')
        text = "TRN_LOGFILES=\""+TRN_LOGFILES["current"]+"\" "+text;

    // these parameters support substitutions
    text += sub_placeholder(x,"input","RESON_HOST",RESON_HOST["current"]);
    text += sub_placeholder(x,"log-directory","TRN_LOGFILES",TRN_LOGFILES["current"]);
    text += sub_placeholder(x,"trn-map","TRN_MAPFILES",TRN_MAPFILES["current"]);
    text += sub_placeholder(x,"trn-par","TRN_DATAFILES",TRN_DATAFILES["current"]);
    text += sub_placeholder(x,"trn-cfg","TRN_DATAFILES",TRN_DATAFILES["current"]);
    text += sub_placeholder(x,"output","SESSION",session_str());
    text += sub_placeholder(x,"mb-out","MBTRN_HOST",MBTRN_HOST["current"]);
    text += sub_placeholder(x,"trn-out","MBTRN_HOST",MBTRN_HOST["current"]);
    text += sub_placeholder(x,"trn-log","TRN_SESSION",trn_session());

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

