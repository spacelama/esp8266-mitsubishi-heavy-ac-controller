/* Handle all the slider translations from values to text */

var slider_temp;
var val_temp_html;
var slider_vdir;
var val_vdir_html;
var slider_hdir;
var val_hdir_html;
var slider_fan;
var val_fan_html;

function refresh_handlers() {
    slider_temp = document.getElementById("slider_temp");
    val_temp_html = document.getElementById("val_temp_html");

    slider_vdir = document.getElementById("slider_vdir");
    val_vdir_html = document.getElementById("val_vdir_html");

    slider_hdir = document.getElementById("slider_hdir");
    val_hdir_html = document.getElementById("val_hdir_html");

    slider_fan = document.getElementById("slider_fan");
    val_fan_html = document.getElementById("val_fan_html");

    slider_temp.oninput = function() {
        set_temp_html(this.value);
    }
    set_temp_html(slider_temp.value);

    slider_vdir.oninput = function() {
        set_vdir_html(this.value);
    }
    set_vdir_html(slider_vdir.value);

    slider_hdir.oninput = function() {
        set_hdir_html(this.value);
    }
    set_hdir_html(slider_hdir.value);

    slider_fan.oninput = function() {
        set_fan_html(this.value);
    }
    set_fan_html(slider_fan.value);
}

function set_temp_html(v) {
    val_temp_html.innerHTML = v;
}

function set_vdir_html(v) {
    switch(6-v) {
    case 0:
        val_vdir_html.innerHTML = "auto/manual";
        break;
    case 1:
        val_vdir_html.innerHTML = "swing";
        break;
    case 2:
        val_vdir_html.innerHTML = "up";
        break;
    case 3:
        val_vdir_html.innerHTML = "mup";
        break;
    case 4:
        val_vdir_html.innerHTML = "middle";
        break;
    case 5:
        val_vdir_html.innerHTML = "mdown";
        break;
    case 6:
        val_vdir_html.innerHTML = "down";
        break;
    default:
        val_vdir_html.innerHTML = "unknown";
        break;
    }
}

function set_hdir_html(v) {
    switch(Number(v)) {
    case 0:
        val_hdir_html.innerHTML = "auto/manual";
        break;
    case 1:
        val_hdir_html.innerHTML = "swing";
        break;
    case 2:
        val_hdir_html.innerHTML = "middle";
        break;
    case 3:
        val_hdir_html.innerHTML = "left";
        break;
    case 4:
        val_hdir_html.innerHTML = "mleft";
        break;
    case 5:
        val_hdir_html.innerHTML = "mright";
        break;
    case 6:
        val_hdir_html.innerHTML = "right";
        break;
    default:
        val_hdir_html.innerHTML = "unknown";
        break;
    }
}

function set_fan_html(v) {
    if (v == 0) {
        val_fan_html.innerHTML = "auto";
    } else {
        val_fan_html.innerHTML = v;
    }
}
