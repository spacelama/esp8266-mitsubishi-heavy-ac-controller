/* Fill in ajax parameters */
// http://www.perlmonks.org/?node_id=628681

function createRequestObject() {
    let req;
    if (window.XMLHttpRequest) { // Firefox, Safari, Opera...
        req = new XMLHttpRequest();
        //    alert('Detected a modern browser - very good!');
    } else if (window.ActiveXObject) { // Internet Explorer 5+
        req = new ActiveXObject("Microsoft.XMLHTTP");
        //    alert('Detected an old IE browser');
    } else {
        // error creating the request object,
        // (maybe an old browser is being used?)
        alert('There was a problem creating the XMLHttpRequest object');
        req = '';
    }
    return req;
}

// http://stackoverflow.com/questions/457549/based-on-how-they-are-constructed-can-callbacks-also-be-defined-as-closures

function sendRequest(params) {
    // Make the XMLHttpRequest object
    let oRequest = createRequestObject();
    if (params=="currenttemp") {
        oRequest.open('GET', 'https://rather.puzzling.org/weewx/data.json');
    } else if (params) {
        // https://stackoverflow.com/questions/9713058/send-post-data-using-xmlhttprequest
        oRequest.open('POST', 'do');
        oRequest.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        resetRefreshTime();
    } else {
        oRequest.open('GET', 'get');
    }
    oRequest.onreadystatechange = this.handleResponse.bind(this,oRequest,"parse_ajax(response);");
    oRequest.send(params);
}

function handleResponse(oResponse,action_response) {
    if(oResponse.readyState == 4) {
        if (oResponse.status == 200){
            let response = oResponse.responseText; // Text returned FROM webserver
            if(response) { // UPDATE ajaxTest content
                //                console.log("Got here3: "+response+"!");
                eval(action_response);
                //                console.log("Got here4!");
            }
        }
    }
}

var power_last;
var mode_last;
var setpointtemp_last;
var vdir_last;
var hdir_last;
var fanspeed_last;
var silent_last;
var _3d_last;
var bgcolour_last;

var bgcolour;

var current_loungeroom_temp;

function populate_power_field(power) {
    let power_str="", power_on_htmlstr="", power_off_htmlstr="";

    switch (power) {
    case 0:
        power_str="off";
        power_off_htmlstr="checked=\"checked\"";
        bgcolour="#777777";
        break;
    case 1:
        power_str="on";
        power_on_htmlstr="checked=\"checked\"";
        break;
    default:
        power_str="unknown";
        bgcolour="#444444";
    }

    if (power != power_last) {
        document.getElementById('ajaxpower').innerHTML=
            "<input type=\"radio\" id=\"0\" name=\"power\" value=\"0\" "+power_off_htmlstr+
            " onchange=\"sendRequest('power='+this.value)\">"+
            "<label for=\"0\">Off</label>"+
            "<input type=\"radio\" id=\"1\" name=\"power\" value=\"1\" "+power_on_htmlstr+
            " onchange=\"sendRequest('power='+this.value)\">"+
            "<label for=\"1\">On</label>";

        power_last=power;
        resetRefreshTime();
    }
}

function populate_mode_field(mode) {
    let mode_str, mode_iscool_htmlstr="", mode_isfan_htmlstr="", mode_isheat_htmlstr="";

    switch (mode) {
    case 3:
        mode_str="cool";
        mode_iscool_htmlstr="selected=\"selected\"";
        if (power_last == 1) {
            // if detect the current temp is such that AC isn't
            // running, make the colour halfway between green (fan)
            // and blue to indicate just blowing air
            if (current_loungeroom_temp > setpointtemp_last) {
                bgcolour="#4444CC";
            } else {
                bgcolour="#448888";
            }
        }
        break;
    case 5:
        mode_str="fan";
        mode_isfan_htmlstr="selected=\"selected\"";
        if (power_last == 1) {
            bgcolour="#44CC44";
        }
        break;
    case 2:
        mode_str="heat";
        mode_isheat_htmlstr="selected=\"selected\"";
        if (power_last == 1) {
            // if detect the current temp is such that heater isn't
            // running, make the colour halfway between green (fan)
            // and red to indicate just blowing air
            if (current_loungeroom_temp < setpointtemp_last) {
                bgcolour="#CC4444";
            } else {
                bgcolour="#888844";
            }
        }
        break;
    default:
        mode_str="unknown";
    }

    if (mode != mode_last) {
        document.getElementById('ajaxmode').innerHTML=
            "<select name=\"mode\" id=\"mode\" onchange=\"sendRequest('mode='+this.value)\">"+
            "<option value=\"cool\" "+mode_iscool_htmlstr+">Cool</option>"+
            "<option value=\"fan\" " +mode_isfan_htmlstr +">Fan</option>" +
            "<option value=\"heat\" "+mode_isheat_htmlstr+">Heat</option>"+
            "</select><br>\n";

        mode_last=mode;
        resetRefreshTime();
    }
}


function populate_temp_field(setpointtemp) {
    if (setpointtemp != setpointtemp_last) {
        document.getElementById('ajaxsetpointtemp').innerHTML=
            "<input type=\"range\" id=\"slider_temp\" name=\"temp\" min=\"18\" max=\"30\" value="+
            setpointtemp+
            "  onmouseup=\"sendRequest('temp='+this.value)\""+
            " ontouchend=\"sendRequest('temp='+this.value)\">";

        setpointtemp_last=setpointtemp;
        resetRefreshTime();
    }
}

function populate_vdir_field(vdir) {
    vdir=6-vdir;
    if (vdir != vdir_last) {
        //https://stackoverflow.com/questions/15935837/how-to-display-a-range-input-slider-vertically - but making it look OK would be a challenge
        document.getElementById('ajaxvdir').innerHTML=
            "<input type=\"range\" id=\"slider_vdir\" name=\"vdir\" min=\"0\" max=\"6\" value="+
            vdir+
        //style=\"-webkit-transform: rotate(90deg);-moz-transform: rotate(90deg);-o-transform: rotate(90deg);-ms-transform: rotate(90deg);transform: rotate(90deg);\"
            "  onmouseup=\"sendRequest('vdir='+(6-this.value))\""+
            " ontouchend=\"sendRequest('vdir='+(6-this.value))\">";

        vdir_last=vdir;
        resetRefreshTime();
    }
}

function populate_hdir_field(hdir) {
    if (hdir != hdir_last) {
        document.getElementById('ajaxhdir').innerHTML=
            "<input type=\"range\" id=\"slider_hdir\" name=\"hdir\" min=\"0\" max=\"6\" value="+
            hdir+
            "  onmouseup=\"sendRequest('hdir='+this.value)\""+
            " ontouchend=\"sendRequest('hdir='+this.value)\">";
        hdir_last=hdir;
        resetRefreshTime();
    }
}

function populate_fan_field(fanspeed) {
    if (fanspeed != fanspeed_last) {
        let fanspeed_str="";
        switch (fanspeed) {
        case 0:
            fanspeed_str="auto";
            break;
        default:
            fanspeed_str=String(fanspeed);
        }

        document.getElementById('ajaxfanspeed').innerHTML=
            "<input type=\"range\" id=\"slider_fan\" name=\"fan\" min=\"0\" max=\"3\" value="+
            fanspeed+
            "  onmouseup=\"sendRequest('fan='+this.value)\""+
            " ontouchend=\"sendRequest('fan='+this.value)\">";

        fanspeed_last=fanspeed;
        resetRefreshTime();
    }
}

function populate_silent_field(silent) {
    let silent_str="", silent_on_htmlstr="", silent_off_htmlstr="";

    if (silent != silent_last) {
        switch (silent) {
        case false:
            silent_str="normal";
            silent_off_htmlstr="checked=\"checked\"";
            break;
        case true:
            silent_str="silent";
            silent_on_htmlstr="checked=\"checked\"";
            break;
        default:
            silent_str="unknown";
        }

        document.getElementById('ajaxsilent').innerHTML=
            "<input type=\"radio\" id=\"0\" name=\"silent\" value=\"0\" "+silent_off_htmlstr+
            " onchange=\"sendRequest('silent='+this.value)\">"+
            "<label for=\"0\">Normal</label>"+
            "<input type=\"radio\" id=\"1\" name=\"silent\" value=\"1\" "+silent_on_htmlstr+
            " onchange=\"sendRequest('silent='+this.value)\">"+
            "<label for=\"1\">Silent</label>";

        silent_last=silent;
        resetRefreshTime();
    }
}

function populate_3d_field(_3d) {
    let _3d_str="", _3d_on_htmlstr="", _3d_off_htmlstr="";

    if (_3d != _3d_last) {
        switch (_3d) {
        case false:
            _3d_str="normal";
            _3d_off_htmlstr="checked=\"checked\"";
            break;
        case true:
            _3d_str="3d";
            _3d_on_htmlstr="checked=\"checked\"";
            break;
        default:
            _3d_str="unknown";
        }

        document.getElementById('ajax3d').innerHTML=
            "<input type=\"radio\" id=\"0\" name=\"3d\" value=\"0\" "+_3d_off_htmlstr+
            " onchange=\"sendRequest('3d='+this.value)\">"+
            "<label for=\"0\">Normal</label>"+
            "<input type=\"radio\" id=\"1\" name=\"3d\" value=\"1\" "+_3d_on_htmlstr+
            " onchange=\"sendRequest('3d='+this.value)\">"+
            "<label for=\"1\">3D</label>";

        _3d_last=_3d;
        resetRefreshTime();
    }
}

var last_updated=0;
function parse_ajax(response) {
    let obj = JSON.parse(response);
    if (typeof obj.TLR !== 'undefined') {
        current_loungeroom_temp=obj.TLR;
        document.getElementById('ajaxtemp').innerHTML=obj.TLR;
    } else {
        populate_power_field(obj.power);
        populate_temp_field(obj.setpointtemp);
        populate_mode_field(obj.mode);
        populate_vdir_field(obj.vdir);
        populate_hdir_field(obj.hdir);
        populate_fan_field(obj.fanspeed);
        populate_silent_field(obj.silent);
        populate_3d_field(obj['3d']);

        if (bgcolour != bgcolour_last) {
            document.body.style.backgroundColor=bgcolour;
            bgcolour_last = bgcolour;
        }
        last_updated=Date.now();
        var locale = window.navigator.userLanguage || window.navigator.language;
        var d = new Date();
        var n = d.toLocaleString(locale);
        document.getElementById('ajaxtime').innerHTML=n;

        refresh_handlers();
    }
}

var timer;
var refresh_time=1;
function resetRefreshTime() {
    refresh_time=1;
    setRefreshTimer();
}
function setRefreshTimer() {
    //        console.log("resettimer:"+resettimer);
    if (timer) {
        clearInterval(timer);
    }
    timer = setInterval('refreshStatus()', refresh_time*1000);
    // Exponential backoff after changes.
    // Double the refresh time every time from 1 second at
    // page load upwards to max of 60 seconds - quick response
    // to immediate changes followed by few queries after
    // those changes take effect, to be mobile-friendly
    refresh_time=refresh_time*2;
    if (refresh_time>60) {
        refresh_time=60;
    }
}
function refreshStatus() {
    let date=Date.now();
    interval=(date-last_updated)/1000;
    if (interval > 120) {
        // grey out stale data
        bgcolour="#222222";
        document.body.style.backgroundColor=bgcolour;
        bgcolour_last = bgcolour;
    }
    setRefreshTimer();
    sendRequest("currenttemp");
    sendRequest();
}
window.onload=function() {
    refreshStatus();
};
