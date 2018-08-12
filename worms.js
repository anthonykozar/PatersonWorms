var edges;
var DEBUG = false;
var DIR_MATRIX = [[-1, 0], [0, -1], [1, -1], [1, 0], [0, 1], [-1, 1]];

var field_suffixes = ["1", "2", "3_1", "3_2", "3_3", "3_4", "4"];
var field1 = 1;
var field2 = 2;
var field3_1 = 2;
var field3_2 = 1;
var field3_3 = 0;
var field3_4 = 2;
var field4 = 0;

var field_array = [field1, field2, field3_1, field3_2, field3_3, field3_4, field4];

createTable();

var snap = Snap();
var snap_center_x;
var snap_center_y;
var line_length;
var stroke_width;
var speed = 0.25;
var zoom_speed = 0.25
var zoom;
var group;
var timer;
var resize_timer;
var stroke_width_slider;
/* addVertex
** x - An x value on the lattice
** y - A y value on the lattice
**
** edges[x] gives the line associated with the x coordinate x on the lattice.
** edges[x][y] gives the vertex (x,y) on the lattice.
** In order to utilize associated arrays, edges is made as a new Object().
** Similarly, each line edges[x] must be initialized as a new Object() if it is not already one.
** The use of a boolean array of length six is that each boolean represents the status of one of the paths
** coming from the vertex. Initially, all paths to the new vertex are uneaten. Booleans take up less
** memory than integers and javascript will allow addition of booleans to integers.
** If the vertex has already been initialized, this function effectively does nothing.
*/
function addVertex(x, y) {
  if(edges[x] == undefined) {
    edges[x] = new Object();
  }
  if(edges[x][y] == undefined) {
    edges[x][y] = [false, false, false, false, false, false];
    if(DEBUG) {
      console.log("addVertex(" + x + ", " + y + ")");
    };
  }
  else {
    if(DEBUG) {
      console.log("addVertex(" + x + ", " + y + ") NOT ADDED");
    }
  }
}

function moveTo(c_x, c_y, x, y, to_dir, step) {
  if(DEBUG) {
    console.log("moveTo(" + c_x + ", " + c_y + ", " + x + ", " +  y + ", " + to_dir + ")");
  };
  
  addVertex(x,y);
  edges[x][y][((to_dir + 3) % 6)] = true;
  edges[c_x][c_y][to_dir] = true;
  
  
  var x1 = snap_center_x;
  var x2 = snap_center_x - line_length*Math.cos(-to_dir*Math.PI/3);
  var y1 = snap_center_y;
  var y2 = snap_center_y - line_length*Math.sin(-to_dir*Math.PI/3);
  var line = snap.line(x1, y1, x2, y2);
  snap_center_x = x2;
  snap_center_y = y2;
  line.attr({
    strokeWidth: stroke_width,
    stroke: "#ff0000",
    strokeLinecap: "round",
  });

  group.add(line);
  for(var i = 0; i < 256; i++) {
    if(step - 4*i >= 0 && group[step-4*i] != undefined) {
      var red = 0xff - i * 0x01;
      group[step-4*i].attr({
        stroke: "#" + ("0" + red.toString(16)).substr(-2) + "0000"
      });
    }
  }
  //check if the line is close enough to the boundary to necessitate a shrink
  var bbox = group.getBBox();
  var vb = snap.attr("viewBox").vb.split(" ");
  var max_x = parseInt(vb[0]) + parseInt(vb[2]);
  var max_y = parseInt(vb[1]) + parseInt(vb[3]);

  if(max_x - bbox.x2 < 0.10*max_x || max_y - bbox.y2 < 0.10*max_y || -max_x - bbox.x > -0.10*max_x || -max_y - bbox.y > -0.10*max_y)
    {
      if(DEBUG) {
        console.log(bbox);
        console.log(vb);
      }
      var steps_zoom = 100;
      timer = setTimeout(function(){zoomOut(zoom, 1, steps_zoom, zoom*1.3, step, x, y, to_dir)}, zoom_speed);
      return true;
    }
  return false;
}

function zoomOut(c_zoom, c_step, steps_zoom, max_zoom, step, cx, cy, cd) {
  if(DEBUG) {
    console.log("zoomOut(" + c_zoom + ", " + c_step + ", " + steps_zoom + ", " +  max_zoom + ", " + step + ", " + cx + ", " +  cx + ", " + cy + ", " + cd + ")");
  }
  zoom = c_zoom+(c_step*(max_zoom-c_zoom)/steps_zoom);
  snap.attr({viewBox: (-window.innerWidth*zoom/2) + " " + (-window.innerHeight*zoom/2) + " " + window.innerWidth*zoom + " " + window.innerHeight*zoom});
  if(c_step < steps_zoom)
    timer = setTimeout(function(){zoomOut(c_zoom, c_step+1, steps_zoom, max_zoom, step, cx, cy, cd)}, zoom_speed);
  else
    timer = setTimeout(function(){nextStep(step, cx, cy, cd);}, speed);
}

function determineMove(c_x, c_y, c_dir, step) {
  if(DEBUG) {
    console.log("determineMove(" + c_x + "," + c_y + "," + c_dir + ")");
  }
  c_edges = edges[c_x][c_y];
  if(DEBUG) {
    console.log("c_edges:\t" + c_edges);
  }
  //check in order of preference
  var x = c_x;
  var y = c_y;
  var new_dir = (c_dir + 3) % 6;
  var eaten = 0;
  for(var i = 0; i < c_edges.length; i++){eaten += c_edges[i]}

  var choice = 1;
  //Determine the correct field of choice
  if(eaten == 1)
    choice += field_array[0];
  else if(eaten == 2)
    choice += field_array[1];
  else if(eaten == 3) {
    //check orientation of the true (eaten) paths
    if((c_edges[(new_dir + 5) % 6] && c_edges[(new_dir + 4) % 6]) ||
      (c_edges[(new_dir + 5) % 6] && c_edges[(new_dir + 3) % 6]))
      choice += field_array[2];
    else if((c_edges[(new_dir + 3) % 6] && c_edges[(new_dir + 4) % 6]) ||
      (c_edges[(new_dir + 1) % 6] && c_edges[(new_dir + 5) % 6]))
      choice += field_array[3];
    else if((c_edges[(new_dir + 2) % 6] && c_edges[(new_dir + 3) % 6]) ||
      (c_edges[(new_dir + 2) % 6] && c_edges[(new_dir + 4) % 6]))
      choice += field_array[4];
    else if((c_edges[(new_dir + 1) % 6] && c_edges[(new_dir + 2) % 6]) ||
      (c_edges[(new_dir + 1) % 6] && c_edges[(new_dir + 3) % 6]))
      choice += field_array[5];
    else {
      choice = 0;
      console.log("UNEXPECTED CHOICE");
    }
  }
  else if(eaten == 4)
    choice += field_array[6];
  
  
  var count = 0;
  while(choice != 0 && count != 5) {
    count += 1;
    choice -= !c_edges[(new_dir+count) % 6];
  }
  if(choice == 0) {
    new_dir = (new_dir+count) % 6;
    x += DIR_MATRIX[new_dir][0];
    y += DIR_MATRIX[new_dir][1];
    
    var zooming = moveTo(c_x, c_y, x, y, new_dir, step);
    if(zooming)
      return false;
    return [x, y, new_dir];
  }
  return false;
}


/* nextStep
** cx - Current x value on the lattice
** cy - Current y value on the lattice
** cd - Current direction the worm is facing
**
** updated_pos will be of the form [new_x, new_y, new_direction]
** If updated_pos is false, then the worm has died or a zoom out is needed.
** If there are still more steps that need to be taken, then the next step should be queued up.
*/
function nextStep(step, cx, cy, cd){  
    var updated_pos = determineMove(cx, cy, cd, step);
    if(updated_pos == false) {
      return false;
    };
    timer = setTimeout(function(){nextStep(step+1, updated_pos[0], updated_pos[1], updated_pos[2]);}, speed);
}


function submitNewWorm() {
  var new_fields = [];
  for(var i = 0; i < field_suffixes.length; i++) {
    new_fields[i] = document.querySelector('input[name = "field' + field_suffixes[i] + '"]:checked');
    if(new_fields[i] == null) {
      alert("Pick a selection for field" + field_suffixes[i]);
      return false;
    }
  }

  clearTimeout(timer);
  snap.clear();

  for(var i = 0; i < field_array.length; i++) {
    field_array[i] = parseInt(new_fields[i].value);
  }
  initWorm();
}

function createTable() {
  var table = document.createElement("TABLE");
  var body = document.getElementsByTagName("body")[0];
  body.appendChild(table);

  var header_row = document.createElement("TR");
  for(var i = 0; i < 5; i++) {
    var header = document.createElement("TH");
    if(i > 0)
      header.innerHTML = ['a', 'b', 'c', 'd'][i-1];
    header_row.appendChild(header);
  }
  table.appendChild(header_row);

  var choice_field = [2, 4, 3, 3, 3, 3, 2];
  for(var j = 0; j < choice_field.length; j++) {
    var tr = document.createElement("TR");
    table.appendChild(tr);
    for(var i = 0; i < choice_field[j] + 1; i++) {
      var td = document.createElement("TD");
      if(i > 0) {
        var x = document.createElement("INPUT");
        x.setAttribute("type", "radio");
        x.setAttribute("name", "field" + field_suffixes[j]);
        x.setAttribute("value", i-1);
        if(field_array[j] == i-1)
          x.checked = true;
        td.appendChild(x);
      }
      else {
        td.innerHTML = "field" + field_suffixes[j];
      }
      tr.appendChild(td);
    }
  }
  
  var submit_button = document.createElement("BUTTON");
  submit_button.setAttribute("onclick", "submitNewWorm()");
  submit_button.innerHTML = "Submit"
  body.appendChild(submit_button);

  body.appendChild(document.createElement("br"));

  stroke_width_slider = document.createElement("INPUT");
  stroke_width_slider.setAttribute("type", "range");
  stroke_width_slider.setAttribute("min", 1);
  stroke_width_slider.setAttribute("max", 20);
  stroke_width_slider.innerHTML = "Stroke Width";
  stroke_width_slider.oninput = function() {
    clearTimeout(resize_timer);
    resize_timer = setTimeout(function() {
      stroke_width = stroke_width_slider.value;
      var i = 0;
      while(group[i] != undefined && group[i].attr("strokeWidth") != (stroke_width + "px")) {
        group[i].attr({strokeWidth: stroke_width});
        i++;
      }
    }, 3000);
  }
  body.appendChild(stroke_width_slider);

  speed_slider = document.createElement("INPUT");
  speed_slider.setAttribute("type", "range");
  speed_slider.setAttribute("id", "speed_slider");
  speed_slider.setAttribute("min", 0.25);
  speed_slider.setAttribute("max", 1000);
  speed_slider.innerHTML = "Speed";
  speed_slider.value = speed;
  speed_slider.oninput = function() {
    speed = parseFloat(speed_slider.value);
  }
  body.appendChild(speed_slider);
}

function fixBounds() {
    snap.attr({viewBox: (-window.innerWidth*zoom/2) + " " + (-window.innerHeight*zoom/2) + " " + window.innerWidth*zoom + " " + window.innerHeight*zoom});
}

/* INITIAL STEPS
** Initially, the worm should move directly to the left.
** This is how it is done by Gardner. Fixing this initial movement halts simple rotations.
** Though this is just a convention and could be changed, I have left it in for consistency.
*/
function initWorm() {
  edges = new Object();
  snap.attr({viewBox: (-window.innerWidth/2) + " " + (-window.innerHeight/2) + " " + window.innerWidth + " " + window.innerHeight, onresize: "fixBounds()"});
  snap_center_x = 0;
  snap_center_y = 0;
  line_length = 10;
  zoom = 1.0;
  group = snap.g();

  addVertex(0,0);
  moveTo(0, 0, -1, 0, 0, 1);
  // Start "moving" the worm
  timer = setTimeout(function(){
    nextStep(2, -1, 0, 0);}, speed);
}

initWorm();