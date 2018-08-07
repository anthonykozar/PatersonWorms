var edges = new Object();
var DEBUG = false;
var snap = Snap();
var snap_center_x = Math.round(window.innerWidth/2);
var snap_center_y = Math.round(window.innerHeight/2);
var line_length = 10;

var group = snap.g();

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

function moveTo(c_x, c_y, x, y, to_dir) {
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
  //if x1 or x2 is greater than .95*svg width or less than 0.05*svg width
  //or if y1 or y2 is greater than .95*svg height or less than 0.05*svg height
  
  if(x1 > 0.95*window.innerWidth || x2 > 0.95*window.innerWidth
    || x1 < 0.05*window.innerWidth || x2 < 0.05*window.innerWidth
    || y1 > 0.95*window.innerHeight || y2 > 0.95*window.innerHeight
    || y1 < 0.05*window.innerHeight || y2 < 0.05*window.innerHeight)
    {
      group.animate({transform: 's0.80,0.80'}, 3000, mina.easein());
    }
};

function determineMove(c_x, c_y, c_dir) {
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
    moveTo(c_x, c_y, x, y, new_dir);
    return [x, y, new_dir];
  }
  return false;
}



var current_x = 0;
var current_y = 0;
var current_dir = 0;

addVertex(0,0);

moveTo(current_x, current_y, -1, 0, 0);
current_x = -1;
var s = 0;
while(s < 3000) {
  var updated_pos = determineMove(current_x, current_y, current_dir);
  if(updated_pos == false) {
    break;
  };
  current_x = updated_pos[0];
  current_y = updated_pos[1];
  current_dir = updated_pos[2];
  s = s+1;
};
console.log("DONE " + s);