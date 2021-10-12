/* The main app logic.
*/

   var MAX_WIDTH = 1000;	// Width of border, doesn't affect calculations
   var MAX_HEIGHT = 1000;	// Height of border, doesn't affect calculations
   var MARKER_SIZE = 8;		// Size of the start/end markers
   var MARKER_START_COL = "blue";
   var MARKER_END_COL = "red";
   var ctx;			// Canvas context for drawing

   var nofly;			// Store the no-fly zone data
   var flightplan;		// Store the flight plan data

   function init(theNoFlyZones, theFlightPlan)
   {
      console.log("init entered");

      nofly = theNoFlyZones;
      flightplan = theFlightPlan;

      window.addEventListener('resize', on_resize);
      on_resize();
      console.log("init exited");
   }

   function on_resize()
   {
      // resize for current window size
      var ctx = document.getElementById("canvas").getContext("2d");
      var cWidth = window.innerWidth;
      var cHeight = window.innerHeight;
      ctx.canvas.width = cWidth;
      ctx.canvas.height = cHeight;
      redraw();
   }

   function circle(x, y, r)
   {
      ctx.fillStyle = "white";
      ctx.strokeStyle = "black";
      ctx.beginPath();
      ctx.arc(x, y, r, 0, Math.PI*2, false);
      ctx.stroke();
   }

   function marker(x, y, size, col)
   {
      var hs = size / 2;
      ctx.fillStyle = col;
      ctx.fillRect(x - hs, y - hs, size, size);
   }

   function drawNoFly()
   {
      nofly.forEach(function(zone) {
	 circle(zone.x, zone.y, zone.r);
      });
   }

   function drawFlightPlan()
   {
      var fpStart = flightplan[0];
      marker(fpStart.x, fpStart.y, MARKER_SIZE, MARKER_START_COL);

      ctx.fillStyle = "white";
      ctx.beginPath();
      ctx.moveTo(fpStart.x, fpStart.y);
      for(var i = 1; i < flightplan.length; i++)
      {
	 var waypoint = flightplan[i];
	 ctx.lineTo(waypoint.x, waypoint.y);
      }
      ctx.stroke();

      var fpEnd = flightplan[flightplan.length - 1];
      marker(fpEnd.x, fpEnd.y, MARKER_SIZE, MARKER_END_COL);  
   }

   function redraw()
   {
      ctx = document.getElementById("canvas").getContext("2d");
      var cWidth = ctx.canvas.width;
      var cHeight = ctx.canvas.height;

      // draw the border
      ctx.fillStyle = "white";
      ctx.strokeStyle = "black";
      ctx.clearRect(0, 0, MAX_WIDTH, MAX_HEIGHT);
      ctx.strokeRect(0, 0, MAX_WIDTH, MAX_HEIGHT);

      drawNoFly();

      drawFlightPlan();
   }
