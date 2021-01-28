/* Handle all the slider translations from values to text */
var slider_temp = document.getElementById("slider_temp");
var val_temp_html = document.getElementById("val_temp_html");
function set_temp_html(v) {
    val_temp_html.innerHTML = v;
}
slider_temp.oninput = function() {
    set_temp_html(this.value);
}
set_temp_html(slider_temp.value);

var slider_vdir = document.getElementById("slider_vdir");
var val_vdir_html = document.getElementById("val_vdir_html");
function set_vdir_html(v) {
    switch(v) {
    case "0":
        val_vdir_html.innerHTML = "auto/manual";
        break;
    case "1":
        val_vdir_html.innerHTML = "swing";
        break;
    case "2":
        val_vdir_html.innerHTML = "up";
        break;
    case "3":
        val_vdir_html.innerHTML = "mup";
        break;
    case "4":
        val_vdir_html.innerHTML = "middle";
        break;
    case "5":
        val_vdir_html.innerHTML = "mdown";
        break;
    case "6":
        val_vdir_html.innerHTML = "down";
        break;
    default:
        val_vdir_html.innerHTML = "unknown";
        break;
    }
}
slider_vdir.oninput = function() {
    set_vdir_html(this.value);
}
set_vdir_html(slider_vdir.value);

var slider_hdir = document.getElementById("slider_hdir");
var val_hdir_html = document.getElementById("val_hdir_html");
function set_hdir_html(v) {
    switch(v) {
    case "0":
        val_hdir_html.innerHTML = "auto/manual";
        break;
    case "1":
        val_hdir_html.innerHTML = "swing";
        break;
    case "2":
        val_hdir_html.innerHTML = "middle";
        break;
    case "3":
        val_hdir_html.innerHTML = "left";
        break;
    case "4":
        val_hdir_html.innerHTML = "mleft";
        break;
    case "5":
        val_hdir_html.innerHTML = "mright";
        break;
    case "6":
        val_hdir_html.innerHTML = "right";
        break;
    default:
        val_hdir_html.innerHTML = "unknown";
        break;
    }
}
slider_hdir.oninput = function() {
    set_hdir_html(this.value);
}
set_hdir_html(slider_hdir.value);

var slider_fan = document.getElementById("slider_fan");
var val_fan_html = document.getElementById("val_fan_html");
function set_fan_html(v) {
    if (v == 0) {
        val_fan_html.innerHTML = "auto";
    } else {
        val_fan_html.innerHTML = v;
    }
}
slider_fan.oninput = function() {
    set_fan_html(this.value);
}
set_fan_html(slider_fan.value);
