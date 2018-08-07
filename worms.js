var edges = new Object();
var DEBUG = false;
var snap = Snap().attr({viewBox: Math.round(-window.innerWidth/2) + " " + Math.round(-window.innerHeight/2) + " " + window.innerWidth + " " + window.innerHeight});
var snap_center_x = 0
var snap_center_y = 0;
var line_length = 10;

var group = snap.g();
var zoom = 1.0;
var field1 = 1;
var field2 = 2;
var field3_1 = 2;
var field3_2 = 1;
var field3_3 = 0;
var field3_4 = 2;
var field4 = 0;

function addVertex(x, y) {
  //edges[x,y] = [] all zero except approaching dir (currentDir)
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
};

function moveTo(c_x, c_y, x, y, to_dir, step) {
  //add in a new vertex
  //update new edge to be taken
  //update old edge to be taken
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
};

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
    if(new_dir == 4)
      y += 1;
    else if(new_dir == 5) {
      x -= 1;
      y += 1;
    }
    else if(new_dir == 0)
      x -= 1;
    else if(new_dir == 1)
      y -= 1;
    else if (new_dir == 2) {
      x += 1;
      y -= 1;
    }
    else if (new_dir == 3)
      x += 1;
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