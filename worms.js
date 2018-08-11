var edges = new Object();
var DEBUG = false;
var DIR_MATRIX = [[-1, 0], [0, -1], [1, -1], [1, 0], [0, 1], [-1, 1]];

var zoom = 1.0;
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

var snap = Snap().attr({viewBox: Math.round(-window.innerWidth/2) + " " + Math.round(-window.innerHeight/2) + " " + window.innerWidth + " " + window.innerHeight});
var snap_center_x = 0;
var snap_center_y = 0;
var line_length = 10;
var group = snap.g();

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
  var x2 = snap_center_x - Math.round(line_length*Math.cos(-to_dir*Math.PI/3));
  var y1 = snap_center_y;
  var y2 = snap_center_y - Math.round(line_length*Math.sin(-to_dir*Math.PI/3));
  var line = snap.line(x1, y1, x2, y2);
  snap_center_x = x2;
  snap_center_y = y2;
  line.attr({
    strokeWidth: 5,
    stroke: "#f00",
    strokeLinecap: "round"
  });

  group.add(line);
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
      setTimeout(function(){zoomOut(zoom, 1, steps_zoom, zoom*1.5, step, x, y, to_dir)}, 0.25);
      zoom *= 1.5;
      return true;
    }
  return false;
}

function zoomOut(c_zoom, c_step, steps_zoom, max_zoom, step, cx, cy, cd) {
  if(DEBUG) {
    console.log("zoomOut(" + c_zoom + ", " + c_step + ", " + steps_zoom + ", " +  max_zoom + ", " + step + ", " + cx + ", " +  cx + ", " + cy + ", " + cd + ")");
  }
  snap.attr({viewBox: Math.round(-window.innerWidth*(c_zoom+(c_step*(max_zoom-c_zoom)/steps_zoom))/2) + " " + Math.round(-window.innerHeight*(c_zoom+(c_step*(max_zoom-c_zoom)/steps_zoom))/2) + " " + window.innerWidth*(c_zoom+(c_step*(max_zoom-c_zoom)/steps_zoom)) + " " + window.innerHeight*(c_zoom+(c_step*(max_zoom-c_zoom)/steps_zoom))});
  if(c_step < steps_zoom)
    setTimeout(function(){zoomOut(c_zoom, c_step+1, steps_zoom, max_zoom, step, cx, cy, cd)}, 0.25);
  else
    setTimeout(function(){nextStep(step, cx, cy, cd);}, 0.25);
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
    choice += field1;
  else if(eaten == 2)
    choice += field2;
  else if(eaten == 3) {
    //check orientation of the true (eaten) paths
    if((c_edges[(new_dir + 5) % 6] && c_edges[(new_dir + 4) % 6]) ||
      (c_edges[(new_dir + 5) % 6] && c_edges[(new_dir + 3) % 6]))
      choice += field3_1;
    else if((c_edges[(new_dir + 3) % 6] && c_edges[(new_dir + 4) % 6]) ||
      (c_edges[(new_dir + 1) % 6] && c_edges[(new_dir + 5) % 6]))
      choice += field3_2;
    else if((c_edges[(new_dir + 2) % 6] && c_edges[(new_dir + 3) % 6]) ||
      (c_edges[(new_dir + 2) % 6] && c_edges[(new_dir + 4) % 6]))
      choice += field3_3;
    else if((c_edges[(new_dir + 1) % 6] && c_edges[(new_dir + 2) % 6]) ||
      (c_edges[(new_dir + 1) % 6] && c_edges[(new_dir + 3) % 6]))
      choice += field3_4;
    else {
      choice = 0;
      console.log("UNEXPECTED CHOICE");
    }
  }
  else if(eaten == 4)
    choice += field4;
  
  
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
    if(step < 4000)
      setTimeout(function(){nextStep(step+1, updated_pos[0], updated_pos[1], updated_pos[2]);}, 0.25);
}


function submitNewWorm() {
  var new_fields = [];
  for(var i = 0; i < field_suffixes.length; i++) {
    new_fields[i] = document.querySelector('input[name = "field' + field_suffixes[i] + '"]:checked');
    console.log(new_fields[i]);
    if(new_fields[i] == null) {
      alert("Pick a selection for field" + field_suffixes[i]);
      return false;
    }
  }
  for(var i = 0; i < field_array.length; i++) {
    field_array[i] = new_fields[i]
  }
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
}
/* INITIAL STEPS
** Initially, the worm should move directly to the left.
** This is how it is done by Gardner. Fixing this initial movement halts simple rotations.
** Though this is just a convention and could be changed, I have left it in for consistency.
*/
addVertex(0,0);
moveTo(0, 0, -1, 0, 0);

// Start "moving" the worm
setTimeout(function(){
  nextStep(0, -1, 0, 0);}, 0.25);