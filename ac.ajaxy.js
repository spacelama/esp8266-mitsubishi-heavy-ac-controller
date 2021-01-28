/* Fill in ajax parameters */
// http://www.perlmonks.org/?node_id=628681

var yet_initialised=false;

function createRequestObject() {
    var req;
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

function sendRequest(action_response,resettimer) {
    // Make the XMLHttpRequest object
    var oRequest = createRequestObject();
    oRequest.open('get', 'get');
    oRequest.onreadystatechange = this.handleResponse.bind(this,oRequest,action_response,resettimer);
    oRequest.send(null);
}

function handleResponse(oResponse,action_response,resettimer) {
    if(oResponse.readyState == 4) {
        if (oResponse.status == 200){
            var response = oResponse.responseText; // Text returned FROM perl script
            if(response) { // UPDATE ajaxTest content
                //                console.log("Got here3: "+response+"!");
                eval(action_response);
                //                console.log("Got here4!");
            }
        }
        //        console.log("resettimer:"+resettimer);
        if (resettimer) {
            setTimeout('refreshstatus()', refresh_time*1000);
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
    }
}

function populate_power_field() {
}

function populate_mode_field() {
}

var setpointtemp_last;

function populate_temp_field(setpointtemp) {
    if (setpointtemp != setpointtemp_last) {
        document.getElementById('ajaxsetpointtemp').innerHTML=
            "<input type=\"range\" id=\"slider_temp\" name=\"temp\" min=\"18\" max=\"30\" value=\""+
            setpointtemp+
            "\" onmouseup=\"this.form.submit()\" ontouchend=\"this.form.submit()\">"
        setpointtemp_last=setpointtemp;
    }
}

function populate_vdir_field() {
}

function populate_hdir_field() {
}

function populate_fan_field() {
}

function populate_silent_field() {
}

function populate_3d_field() {
}

// function inhibit_setpoint_update() {
//     setpoint_adjusted=1;

//     var oRequest = createRequestObject();
//     oRequest.open('get', 'get?************=');
//     oRequest.send(null);
// };

function parse_ajax(response) {
    var obj = JSON.parse(response);
    populate_temp_field(obj.setpointtemp);

//    document.getElementById('ajaxshowstatus').innerHTML=obj.status;
//    document.getElementById('ajaxafterminutestext').innerHTML=obj.aftertogglebutton;
//    document.getElementById('ajaxuntilminutestext').innerHTML=obj.untiltogglebutton;
//    document.getElementById('ajaxshowcurrenttemp').innerHTML=obj.currenttempbutton;

    //FIXME: if detect the current temp is such that AC/heater isn't running, make the colour halfway between green (fan) and red/blue to indicate just blowing air
//    document.body.style.backgroundColor=obj.bgcolour;

//    if (!setpoint_adjusted) {
//        document.getElementById('ajaxshowsetpoint').innerHTML=obj.setpoint;
//    }

    if (!yet_initialised) {
        // now load the library to initialise the input fields that have now been set up:
        // https://stackoverflow.com/questions/5892845/how-to-load-one-javascript-file-from-another
        var newScript = document.createElement('script');
        newScript.type = 'text/javascript';
        //FIXME:
        newScript.src = 'http://rather.puzzling.org/~tconnors/temp-ac.feedback.js';
        document.getElementsByTagName('head')[0].appendChild(newScript);

        yet_initialised=true;
    }
}

function refreshstatus() {
    sendRequest("parse_ajax(response);", 1);
}

setpoint_adjusted=0;
refresh_time=1;

window.onload=function() {
//    document.getElementById('ajaxshowsetpoint').on("input", inhibit_setpoint_update); // FIXME: could add a queue here, and update the live value to the last value in the queue after a small delay.  Then when that's the case, also make an onblur event that enables update, and remove the Setpoint button.  Now all updates via any input method are live, but we don't keep resetting focus every few seconds while trying to adjust the number (mmm, clear inhibit flag only when last bgcolour status gets returned, and when focus isn't current?)
    refreshstatus();
};
