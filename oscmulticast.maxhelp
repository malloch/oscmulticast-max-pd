{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 957.0, 215.0, 535.0, 445.0 ],
		"bglocked" : 0,
		"defrect" : [ 957.0, 215.0, 535.0, 445.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 0,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 0,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print oscmulticast",
					"id" : "obj-3",
					"fontname" : "Arial",
					"patching_rect" : [ 60.0, 255.0, 103.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontsize" : 12.0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "This objects requires the group and port attributes to be set.",
					"linecount" : 3,
					"id" : "obj-10",
					"fontname" : "Arial",
					"patching_rect" : [ 330.0, 135.0, 150.0, 48.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontsize" : 12.0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"id" : "obj-8",
					"fontname" : "Arial",
					"patching_rect" : [ 60.0, 120.0, 50.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 2,
					"fontsize" : 12.0,
					"outlettype" : [ "float", "bang" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"id" : "obj-6",
					"fontname" : "Arial",
					"patching_rect" : [ 75.0, 225.0, 245.0, 18.0 ],
					"numinlets" : 2,
					"numoutlets" : 1,
					"fontsize" : 12.0,
					"outlettype" : [ "" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "/send_some_data $1",
					"id" : "obj-4",
					"fontname" : "Arial",
					"patching_rect" : [ 60.0, 150.0, 123.0, 18.0 ],
					"numinlets" : 2,
					"numoutlets" : 1,
					"fontsize" : 12.0,
					"outlettype" : [ "" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "oscmulticast @group 224.0.1.3 @port 7570",
					"id" : "obj-1",
					"fontname" : "Arial",
					"patching_rect" : [ 60.0, 180.0, 242.0, 20.0 ],
					"numinlets" : 1,
					"numoutlets" : 1,
					"fontsize" : 12.0,
					"outlettype" : [ "" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "bpatcher",
					"id" : "obj-64",
					"args" : [  ],
					"patching_rect" : [ 315.0, 285.0, 211.0, 145.0 ],
					"numinlets" : 0,
					"name" : "dot.menu.maxpat",
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Sends and receives Open Sound Control over multicast IP. Similar to the mxj objects net.multi.send/recv but doesn't garble OSC messages.",
					"linecount" : 2,
					"id" : "obj-16",
					"fontname" : "Arial",
					"patching_rect" : [ 15.0, 45.0, 462.0, 39.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"fontsize" : 14.0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_title",
					"text" : "oscmulticast",
					"id" : "obj-22",
					"fontname" : "Arial",
					"textcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"patching_rect" : [ 15.0, 15.0, 485.0, 30.0 ],
					"numinlets" : 1,
					"frgb" : [ 1.0, 1.0, 1.0, 1.0 ],
					"numoutlets" : 0,
					"fontface" : 3,
					"fontsize" : 20.871338
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"id" : "obj-60",
					"patching_rect" : [ 501.0, 15.0, 4.0, 304.0 ],
					"numinlets" : 1,
					"numoutlets" : 0,
					"bgcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"varname" : "autohelp_top_panel[1]",
					"grad1" : [ 0.0, 0.0, 0.0, 1.0 ],
					"background" : 1,
					"angle" : 180.0,
					"grad2" : [ 1.0, 0.0, 0.0, 1.0 ],
					"id" : "obj-2",
					"patching_rect" : [ 15.0, 15.0, 490.0, 31.0 ],
					"numinlets" : 1,
					"mode" : 1,
					"numoutlets" : 0
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-4", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-4", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-6", 1 ],
					"hidden" : 0,
					"midpoints" : [ 69.5, 212.0, 310.5, 212.0 ]
				}

			}
 ]
	}

}
