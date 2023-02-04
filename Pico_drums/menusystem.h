
// Copyright 2020 Rich Heslip
//
// Author: Rich Heslip 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// this is adapted from my XVA1 menu system. this one is bitmapped so its a mix of character and pixel addressing
// Feb 2022 - adapted again as a single encoder menu system - very similar to the Arduino Neu-rah menu system but only 2 levels
// menu items are displayed top to bottom of screen with a title bar
// encoder scrolls menu selector, click to select submenu
// encoder scrolls submenu selector, click to edit parameter
// last submenu item is treated as "back to top menu" so make sure its set up that way
// parameters can be:
// text strings which return ordinals
// integers in range -9999 to +9999, range and increment set in the subment table
// floats in range -9.99 to +9.99 - floats are displayed but the parameter behind them is an int in the range -999 to +999 so your code has to convert the in to float
// the parameter field in the submenu initializer must point to an integer variable - when you edit the on screen value its this value you are changing
// the handler field in the submenu initializer must be either null or point to a function which is called when you edit the parameter

#define DISPLAY_X 20  // 20 char display
#define DISPLAY_Y 8   // 8 lines
#define DISPLAY_CHAR_HEIGHT 8 // character height in pixels - for bitmap displays
#define DISPLAY_CHAR_WIDTH 6 // character width in pixels - for bitmap displays
#define DISPLAY_X_MENUPAD 2   // extra space between menu items
#define DISPLAY_Y_MENUPAD 2   // extra vertical space between menu items
#define TOPMENU_LINE 1    // line to start menus on
#define TOPMENU_Y (TOPMENU_LINE*(DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD))   // pixel y position to display top menus
#define TOPMENU_X (1 * DISPLAY_CHAR_WIDTH)   // x pos to display top menus - first character reserved for selector character
#define TOPMENU_LINES 5 // number of menu text lines to display
#define SUBMENU_LINE 1 // line to start sub menus on
#define SUBMENU_Y (SUBMENU_LINE*(DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD))   // line to display sub menus
#define SUBMENU_X (1 * DISPLAY_CHAR_WIDTH)   // x pos to display sub menus name field
#define SUBMENU_VALUE_X (14 * DISPLAY_CHAR_WIDTH)  // x pos to display submenu values
#define SUBMENU_LINES 5 // number of menu text lines to display
#define FILEMENU_LINES 5 // number of files to show 
#define FILEMENU_X (1 * DISPLAY_CHAR_WIDTH)   // x pos to display file menus - first character reserved for selector character
#define FILEMENU_Y (TOPMENU_LINE*(DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD))   // pixel y position to display file menus

const char *menutitle;  // points to title of current menu/submenu
const char *maintitle="   Pico Drums   ";

const char *filesroot="./samples";  // root of file tree

char directory[80]={""}; // current working directory

int8_t topmenuindex=0;  // keeps track of which top menu item we are displaying
//int8_t fileindex=0;  // keeps track of which file we are displaying
int numfiles=0; // number of files in current directory

enum paramtype{TYPE_NONE,TYPE_INTEGER,TYPE_FLOAT, TYPE_TEXT,TYPE_FILENAME}; // parameter display types

// holds file and directory info
struct fileinfo {
	char name[80];
	bool isdir;   // true if directory
} files[10];

	
// submenus 
struct submenu {
  const char *name; // display name
  int16_t min;  // min value of parameter
  int16_t max;  // max value of parameter
  int16_t step; // step size. if 0, don't print ie spacer
  enum paramtype ptype; // how its displayed
  const char ** ptext;   // points to array of text for text display
  int16_t *parameter; // value to modify
  void (*handler)(void);  // function to call on value change
};

// top menus
struct menu {
   const char *name; // menu text
   struct submenu * submenus; // points to submenus for this menu
   int8_t submenuindex;   // stores the index of the submenu we are currently using
   int8_t numsubmenus; // number of submenus - not sure why this has to be int but it crashes otherwise. compiler bug?
};

// dummy variable for menu testing
int16_t dummy;

/*
// reset pitch that was modulated by a CV to a reasonable default value
// saves having to manually do it when you "unmodulate" a parameter
void resetCVpitch(void) {
	if (samp[topmenuindex].pitchCV==0) samp[topmenuindex].pitch=1.0;
}; // 
*/

// show sample name of voice 
// relies on topmenuindex having the same index as the current voice

void printsamplename(void) {
  int y= SUBMENU_Y+DISPLAY_Y_MENUPAD;  
	display.setCursor (SUBMENU_X, y ); // leave room for selector
  display.print("                    "); // erase old
  display.setCursor (SUBMENU_X, y ); // leave room for selector
	char temp[DISPLAY_X];  // chop the name to no more than 20 chars
		      //strncpy(temp,samp[sub[index].min].filename,DISPLAY_X-1); // hokey way of finding the sample's filename
  strncpy(temp,sample[voice[topmenuindex].sample].sname,DISPLAY_X-1);
  strcat(temp,"");
	display.print(temp);  // 	
}; 

void testfunc(void) {
  printf("test function %d\n",dummy);
}; // 

// for sorting filenames in alpha order
int comp(const void *a,const void *b) {
return (strcmp((char *)a,(char *)b));
}
 
// ********** menu structs that build the menu system below *********


// text arrays used for submenu TYPE_TEXT fields
const char * textmode[] = {"Trig", "Loop", "Gated"};
const char * textmidimode[] = {"Off    ", "Percuss", "Notes  "};
const char * modtarget[] = {"Nothing","  Level", "    Pan","  Speed","  Pitch"};
const char * CVchannel[] = {"None","   1", "   2","   3","   4","   5","   6","   7","   8"};

struct submenu sample0params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[0].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[0].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[0].pan,0,              
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu sample1params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[1].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[1].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[1].pan,0,   
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu sample2params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[2].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[2].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[2].pan,0,   
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu sample3params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[3].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[3].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[3].pan,0,   
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu sample4params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[4].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[4].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[4].pan,0,   
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu sample5params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[5].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[5].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[5].pan,0,   
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu sample6params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[6].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[6].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[6].pan,0,   
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu sample7params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,NUM_SAMPLES-1,1,TYPE_FILENAME,0,&voice[7].sample,printsamplename,
  "Level",0,1000,10,TYPE_FLOAT,0,&voice[7].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&voice[7].pan,0,   
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&dummy,0, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

/*

struct submenu sample0params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "",0,0,1,TYPE_FILENAME,0,&dummy,0,          // hokey - value of min is the sample number
  "Play Mode",0,2,1,TYPE_TEXT,textmode,&samp[0].mode,0, 
  "Level",0,1000,10,TYPE_FLOAT,0,&samp[0].level,0,
  "Pan",-1000,1000,10,TYPE_FLOAT,0,&samp[0].pan,0,
  "Speed ",-2000,2000,10,TYPE_FLOAT,0,&samp[0].speed,0,  
  "Transpose ",-24,24,1,TYPE_INTEGER,0,&samp[0].transpose,0,  
  "MIDI Mode",0,2,1,TYPE_TEXT,textmidimode,&samp[0].midimode,0,   
  "MIDI Ch ",1,16,1,TYPE_INTEGER,0,&samp[0].midichannel,0,  
  "TrgNote/Pitch",0,127,1,TYPE_INTEGER,0,&samp[0].note,0,  
  "Level CV",0,8,1,TYPE_TEXT,CVchannel,&samp[0].levelCV,0, 
  "Pan CV",0,8,1,TYPE_TEXT,CVchannel,&samp[0].panCV,0, 
  "Speed CV",0,8,1,TYPE_TEXT,CVchannel,&samp[0].speedCV,0, 
  "Pitch CV",0,8,1,TYPE_TEXT,CVchannel,&samp[0].pitchCV,resetCVpitch, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};


struct submenu systemmenu[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "WiFi",0,1,1,TYPE_TEXT,wifimode,&wifistate,wifionoff, 
  "BACK",0,0,1,TYPE_NONE,0,&dummy,0,
};

struct submenu osc2params[] = {
  // name,min,max,step,type,*textfield,*parameter,*handler
  "Osc. On/Off",0,1,1,TYPE_TEXT,textoffon,&enab1,0,
  "Waveform",0,8,1,TYPE_TEXT,textwaves,&wave1,0,
  "Detune",-10,255,1,TYPE_INTEGER,0,&detune2,0,
  "Level",-100,9999,100,TYPE_INTEGER,0,&level1,0,
  "Pan",0,255,1,TYPE_INTEGER,0,&pan1,0,
  "Osc. On/Off",0,1,1,TYPE_TEXT,textoffon,&enab1,0,
  "Waveform",0,8,1,TYPE_TEXT,textwaves,&wave1,0,
  "Detune",-10,255,1,TYPE_INTEGER,0,&detune2,0,
  "Level",-100,9999,100,TYPE_INTEGER,0,&level1,0,
  "MIDI CH",1,16,1,TYPE_INTEGER,0,&pan1,0,
  "BACK",0,0,1,TYPE_NONE,0,&pan1,0,
};
*/

// top level menu structure - each top level menu contains one submenu
struct menu mainmenu[] = {
  // name,submenu *,initial submenu index,number of submenus
  "1 ",sample0params,0,sizeof(sample0params)/sizeof(submenu),
  "2 ",sample1params,0,sizeof(sample1params)/sizeof(submenu),
  "3 ",sample2params,0,sizeof(sample2params)/sizeof(submenu),
  "4 ",sample3params,0,sizeof(sample3params)/sizeof(submenu),
  "5 ",sample4params,0,sizeof(sample4params)/sizeof(submenu),
  "6 ",sample5params,0,sizeof(sample5params)/sizeof(submenu),
  "7 ",sample6params,0,sizeof(sample6params)/sizeof(submenu),
  "8 ",sample7params,0,sizeof(sample7params)/sizeof(submenu),  
 };

#define NUM_MAIN_MENUS sizeof(mainmenu)/ sizeof(menu)
menu * topmenu=mainmenu;  // points at current menu

// highlight the currently selected menu item
void drawselector( int8_t index) {
  int line = index % TOPMENU_LINES;
  display.setCursor (0, TOPMENU_Y+DISPLAY_Y_MENUPAD+line*(DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD) );
  display.print(">"); 
  display.display();
}

// highlight the currently selected menu item as being edited
void draweditselector( int8_t index) {
  int line = index % TOPMENU_LINES;
  display.setCursor (0, TOPMENU_Y+DISPLAY_Y_MENUPAD+line*(DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD) );
  display.print("*"); 
  display.display();
}

// dehighlight the currently selected menu item
void undrawselector( int8_t index) {
  int line = index % TOPMENU_LINES;
  display.setCursor (0, TOPMENU_Y+DISPLAY_Y_MENUPAD+line*(DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD) );
  display.print(" "); 
  display.display();
}

// display the top menu
// index - currently selected top menu
void drawtopmenu( int8_t index) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("%s",menutitle);
    int i = (index/TOPMENU_LINES)*TOPMENU_LINES; // which group of menu items to display
    int last = i+NUM_MAIN_MENUS % TOPMENU_LINES; // show only up to the last menu item
    if ((i + TOPMENU_LINES) <= NUM_MAIN_MENUS) last = i+TOPMENU_LINES; // handles case like 2nd of 3 menu pages
    int y=TOPMENU_Y+DISPLAY_Y_MENUPAD;

    for (i; i< last ; ++i) {
      display.setCursor ( TOPMENU_X, y ); 
      display.print(topmenu[i].name);
    
	  if (i < NUM_VOICES) {			// items 0-7 are always samples - show the sample filename
		  char temp[DISPLAY_X];  // chop the name to no more than 18 chars
		  strncpy(temp,sample[voice[i].sample].sname,DISPLAY_X-3); // 3 columns are used: selector, sample#, space
          temp[DISPLAY_X-3]=0; // null terminate
		  display.print(temp);  // 
	  }
    
      y+=DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD;
    }
    display.display();
} 

// display a sub menu item and its value
// index is the index into the current top menu's submenu array

void drawsubmenu( int8_t index) {
    submenu * sub;
    sub=topmenu[topmenuindex].submenus; //get pointer to the submenu array
    // print the name text
    int y= SUBMENU_Y+DISPLAY_Y_MENUPAD +(index % SUBMENU_LINES)*(DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD); // Y position of this menu index
    display.setCursor (SUBMENU_X,y) ; // set cursor to parameter name field
    display.print(sub[index].name); 
    
    // print the value
    display.setCursor (SUBMENU_VALUE_X, y ); // set cursor to parameter value field
    display.print("      "); // erase old value
    display.setCursor (SUBMENU_VALUE_X, y ); // set cursor to parameter value field
    if (sub[index].step !=0) { // don't print dummy parameter 
      int16_t val=*sub[index].parameter;  // fetch the parameter value
      char temp[5];
      switch (sub[index].ptype) {
        case TYPE_INTEGER:   // print the value as an unsigned integer    
          sprintf(temp,"%4d",val); // lcd.print doesn't seem to print uint8 properly
          display.print(temp);  
          display.print(" ");  // blank out any garbage
          break;
        case TYPE_FLOAT:   // print the int value as a float  
          sprintf(temp,"%1.2f",(float)val/1000); // menu should have int value between -1000 to +1000 so float is -1 to +1
          display.print(temp);  
          display.print(" ");  // blank out any garbage
          break;
        case TYPE_TEXT:  // use the value to look up a string
          if (val > sub[index].max) val=sub[index].max; // sanity check
          if (val < 0) val=0; // min index is 0 for text fields
          display.print(sub[index].ptext[val]); // parameter value indexes into the string array
          display.print(" ");  // blank out any garbage
          break;
		    case TYPE_FILENAME:  // print filename of sample using index in min
		      display.setCursor (SUBMENU_X, y ); // leave room for selector
		      char temp[DISPLAY_X];  // chop the name to no more than 20 chars
		      //strncpy(temp,samp[sub[index].min].filename,DISPLAY_X-1); 
          strncpy(temp,sample[voice[topmenuindex].sample].sname,DISPLAY_X-1); // use topmenuindex to file sample's filename 
          strcat(temp,"");
		      display.print(temp);  // 
        break;
        default:
        case TYPE_NONE:  // blank out the field
          display.print("     ");
          break;
      } 
    }
    display.display(); 
}

// display sub menus of the current topmenu

void drawsubmenus() {
    int8_t index,len;
    index= topmenu[topmenuindex].submenuindex; // submenu field index
    len= topmenu[topmenuindex].numsubmenus; // number of submenu items
    submenu * sub=topmenu[topmenuindex].submenus; //get pointer to the current submenu array
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("     Sample %s",topmenu[topmenuindex].name); // show the menu we came from at top of screen
    int i = (index/SUBMENU_LINES)*SUBMENU_LINES; // which group of menu items to display
    int last = i+len % SUBMENU_LINES; // show only up to the last menu item
    if ((i + SUBMENU_LINES) <= len) last = i+SUBMENU_LINES; // handles case like 2nd of 3 menu pages
    int y=SUBMENU_Y+DISPLAY_Y_MENUPAD;

    for (i; i< last ; ++i) {
      //display.setCursor ( SUBMENU_X, y ); 
      //display.print(sub[i].name);
      //y+=DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD;
      drawsubmenu(i);
    }
    display.display();
} 

/* function to get the content of a given folder */
/*
int get_dir_content(char * path)
{
  DIR * d = opendir(path); // open the path
  if(d==NULL) return 0; // if was not able, return
  struct dirent * dir; // for the directory entries
  int numfiles=0;
  
  while ((dir = readdir(d)) != NULL) // if we were able to read somehting from the directory
    {
      if(dir-> d_type != DT_DIR) {// if the type is not directory just print it with blue color
		strcpy(files[numfiles].name,dir->d_name);
		files[numfiles].isdir=0;
		++ numfiles;
	  }
      else
      if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) // if it is a directory
      {
	   strcpy(files[numfiles].name,dir->d_name);
		files[numfiles].isdir=1;
		++ numfiles;
      }
    }
    closedir(d); // finally close the directory
	qsort (files, numfiles, sizeof(fileinfo), comp);  // put them in alpha order
	strcpy(files[numfiles].name,".."); // last entry is to go up
	files[numfiles].isdir=0; // special case - don't treat it as a directory
	++numfiles;
	return numfiles;
}


// display a list of files
// index - currently selected file
void drawfilelist( int16_t index) {
	char temp[DISPLAY_X+5];  // chop the name to no more than 20 chars
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("S%d /%s",topmenuindex,directory); // show sample # and current directory on top line
    int16_t i = (index/FILEMENU_LINES)*FILEMENU_LINES; // which group of menu items to display
    int last = i+numfiles % TOPMENU_LINES; // show only up to the last menu item
    if ((i + FILEMENU_LINES) <= numfiles) last = i+FILEMENU_LINES; // handles case like 2nd of 3 menu pages
    int y=FILEMENU_Y+DISPLAY_Y_MENUPAD;

    for (i; i< last ; ++i) {
      display.setCursor ( FILEMENU_X, y ); 
	  if (files[i].isdir) display.print("/"); // show its a directory  		
	  strncpy(temp,files[i].name,DISPLAY_X); // make sure it doesn't wrap to next line
	  temp[DISPLAY_X]=0; // null terminate
      display.print(temp);
      y+=DISPLAY_CHAR_HEIGHT+DISPLAY_Y_MENUPAD;
    }
    display.display();
} 
*/

// menu handler
// a run to completion state machine - it never blocks except while waiting for encoder button release
// allows the rest of the application to keep playing audio while parameters are adjusted

enum uimodes{TOPSELECT,SUBSELECT,PARAM_INPUT,FILEBROWSER,WAITFORBUTTONUP}; // UI state machine states


void domenus(void) {
  int16_t enc;
  int8_t index; 
  char temp[80]; // for file paths
  static int16_t fileindex=0;  // index of selected file/directory   
  static int16_t lastdir=0;  // index of last directory we looked at
  static int16_t lastfile=0;  // index of last file we looked at  
  static int16_t uistate=TOPSELECT; // start out at top menu

//  ClickEncoder::Button button; 
  
  enc=Encoder2.getValue();
//  button= Encoder2.getButton();

  // process the menu encoder 
//  enc=P4Encoder.getValue();
//  button= P4Encoder.getButton();

  switch (uistate) {
    case TOPSELECT:  // scrolling thru top menu
      if (enc !=0 ) { // move selector
        int topmenupage = (topmenuindex) / TOPMENU_LINES;  
        undrawselector(topmenuindex);
        topmenuindex+=enc;
        if (topmenuindex <0) topmenuindex=0;  // we don't wrap menus around, just stop at the ends
        if (topmenuindex >=(NUM_MAIN_MENUS -1) ) topmenuindex=NUM_MAIN_MENUS-1; 
        if ((topmenuindex / TOPMENU_LINES) != topmenupage) {
          drawtopmenu(topmenuindex);  // redraw if we scrolled beyond the menu page
        }
        drawselector(topmenuindex);    
      }
      if (!digitalRead(R_SW)) { // menu item has been selected so show submenu
        topmenu[topmenuindex].submenuindex=0;  // start from the first item
        drawsubmenus();
        drawselector(topmenu[topmenuindex].submenuindex);  
        uistate=SUBSELECT;
        while(!digitalRead(R_SW)); // wait till button released
      }
      break;
    case SUBSELECT:  // scroll thru submenus
      if (enc !=0 ) { // move selector
        int submenupage = topmenu[topmenuindex].submenuindex / SUBMENU_LINES;  
        undrawselector(topmenu[topmenuindex].submenuindex);
        topmenu[topmenuindex].submenuindex+=enc;
        if (topmenu[topmenuindex].submenuindex <0) topmenu[topmenuindex].submenuindex=0;  // we don't wrap menus around, just stop at the ends
        if (topmenu[topmenuindex].submenuindex >=(topmenu[topmenuindex].numsubmenus -1) ) topmenu[topmenuindex].submenuindex=topmenu[topmenuindex].numsubmenus -1; 
        if ((topmenu[topmenuindex].submenuindex / SUBMENU_LINES) != submenupage) {
          drawsubmenus();  // redraw if we scrolled beyond the menu page
        }
        drawselector(topmenu[topmenuindex].submenuindex);   
      } 
      if (!digitalRead(R_SW)) { // submenu item has been selected so either go back to top or go to change parameter state
	      submenu * sub;
		    sub=topmenu[topmenuindex].submenus; //get pointer to the submenu array
        if (topmenu[topmenuindex].submenuindex ==(topmenu[topmenuindex].numsubmenus -1)) { // last submenu is always "back to topmenu"
            drawtopmenu(topmenuindex);
            drawselector(topmenuindex); 
            uistate=TOPSELECT;
           while(!digitalRead(R_SW)); // wait till button released
        }
        else {
          undrawselector(topmenu[topmenuindex].submenuindex);
          draweditselector(topmenu[topmenuindex].submenuindex); // show we are editing
          uistate=PARAM_INPUT;  // change the submenu parameter
          while(!digitalRead(R_SW)); // wait till button released
        }
      }        
/*        
		else if (sub[topmenu[topmenuindex].submenuindex].ptype ==TYPE_FILENAME ) { // if filebrowser menu type so we go into file browser
			char temp[80];
			strcpy(temp,filesroot);
			strcat(temp,"/");
			strcat(temp,directory);
			numfiles=get_dir_content(temp);
			fileindex=0;
            if (files[0].isdir) drawfilelist(fileindex=lastdir);
			else drawfilelist(fileindex=lastfile);
            drawselector(fileindex); 
            uistate=FILEBROWSER;
            while(!digitalRead(R_SW)); // wait till button released
        }
        else {
          undrawselector(topmenu[topmenuindex].submenuindex);
          draweditselector(topmenu[topmenuindex].submenuindex); // show we are editing
          uistate=PARAM_INPUT;  // change the submenu parameter
          while(!digitalRead(R_SW)); // wait till button released
        }
      }  
      */ 
      break;
    case PARAM_INPUT:  // changing value of a parameter
      if (enc !=0 ) { // change value
        index= topmenu[topmenuindex].submenuindex; // submenu field index
        submenu * sub=topmenu[topmenuindex].submenus; //get pointer to the current submenu array
        int16_t temp=*sub[index].parameter + enc*sub[index].step; // menu code uses ints - convert to floats when needed
        if (temp < (int16_t)sub[index].min) temp=sub[index].min;
        if (temp > (int16_t)sub[index].max) temp=sub[index].max;
        *sub[index].parameter=temp;
        if (sub[index].handler != 0) (*sub[index].handler)();  // call the handler function
        drawsubmenu(index);
      }
      if (!digitalRead(R_SW)) { // stop changing parameter
        undrawselector(topmenu[topmenuindex].submenuindex);
        drawselector(topmenu[topmenuindex].submenuindex); // show we are selecting again
        uistate=SUBSELECT;
        while(!digitalRead(R_SW)); // wait till button released
      }   
      break;
      /*
	  case FILEBROWSER:  // browse files - file structure is ./<filesroot>/<directory>/<file> ie all files must be in a directory and no more than 1 directory deep
      if (enc !=0 ) { // move selector
        int filespage = (fileindex) / TOPMENU_LINES;  
        undrawselector(fileindex);
        fileindex+=enc;
        if (fileindex <0) fileindex=0;  // we don't wrap menus around, just stop at the ends
        if (fileindex >=(numfiles -1) ) fileindex=numfiles-1; 
        if ((fileindex / TOPMENU_LINES) != filespage) {
          drawfilelist(fileindex);  // redraw if we scrolled beyond the menu page
        }
        drawselector(fileindex);    
      }
      if (!digitalRead(R_SW)) { // file item has been selected 
		    if (files[fileindex].isdir) {  // show nested directory	
			    lastdir=fileindex;    // remember where we were
			    strcpy(temp,filesroot);  // build new file path
			    strcat(temp,"/");
			    strcat(temp,files[fileindex].name);
			    strcpy(directory,files[fileindex].name); // save the current directory
			    numfiles=get_dir_content(temp);
			    fileindex=lastfile;
          drawfilelist(fileindex);
          drawselector(fileindex);
			    while(!digitalRead(R_SW)); // wait till button released
		    }
		  else if (fileindex == (numfiles -1)) { // last file is always ".." so go up to directories
			  numfiles=get_dir_content(filesroot);
			  strcpy(directory,""); // keep track of current directory
			  lastfile=0;        // new directory so start at beginning
        drawfilelist(fileindex=lastdir);
        drawselector(lastdir);
			  while(!digitalRead(R_SW)); // wait till button released
		  }
		  else {  // we have selected a file so load it
			  bool exitflag=0;
			  while (!digitalRead(R_SW)) {   // long press to exit - way to get out of a very long file list
				if (buttoncnt > BUTTON_LONGPRESS) {
					exitflag=1;
					break;
				}
			}
			if (exitflag) {  // don't load file, go back to root dir
				lastfile=0;
				strcpy(directory,"");
			}
			else {
				lastfile=fileindex;  // remember where we were
				char temp2[80];
				strcpy(temp2,filesroot);  // build the full file path
				strcat(temp2,"/");
				strcpy(temp,directory);
				strcat(temp,"/");
				strcat(temp,files[fileindex].name);
				strcpy(samp[topmenuindex].filename,temp); // save directory/filename
				strcat(temp2,temp);
				samp[topmenuindex].state=SUSPENDED;  // turn off access to this sample temporarily
				//printf("loading %s \n",temp2);
				audioFile[topmenuindex].load(temp2); // **** need error checking here for filename
				//audioFile[topmenuindex].printSummary();
				samp[topmenuindex].state=SILENT;  // turn access back on
			}
			topmenu[topmenuindex].submenuindex=0;  // restore submenu from the first item
			drawsubmenus();
			drawselector(topmenu[topmenuindex].submenuindex);  
			uistate=SUBSELECT;	
			while(!digitalRead(R_SW)); // wait till button released
		}
		
    }
        */

    
  } // end of case statement
}


