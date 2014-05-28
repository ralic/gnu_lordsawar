// Copyright (C) 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2004, 2005 Andrea Paternesi
// Copyright (C) 2011, 2012, 2014 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "xmlhelper.h"
#include "defs.h"
#include "File.h"
#include "ucompose.hpp"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

//they are only needed later for the expat callbacks
Glib::ustring my_cdata;
bool error = false;
    
Glib::ustring XML_Helper::xml_entity = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";

// forward declarations of the internally used functions
void start_handler(void* udata, const XML_Char* name, const XML_Char** atts);
void character_handler(void* udata, const XML_Char* s, int len);
void end_handler(void* udata, const XML_Char* name);

XML_Helper::XML_Helper(Glib::ustring filename, std::ios::openmode mode, bool zip)
  : d_inbuf(0), d_outbuf(0), d_fout(0), d_fin(0), d_out(0), d_in(0),
    d_last_opened(""), d_version(""), d_failed(false), d_zip(zip)
{
    debug("Constructor called  -- " << zip)
        
    // always use a helper either for reading or for writing. Doing both
    // is propably possible, but there is little point in using it anyway.
    if ((mode & std::ios::in) && (mode & std::ios::out))
    {
        std::cerr << "XML_Helper: Either open file for reading or writing, not both, exiting\n";
        exit(-1);
    }

    //open input stream if required
    if (mode & std::ios::in)
    {
        d_fin = new std::ifstream(filename.c_str(), std::ios::in);

        if (!(*d_fin))
        //error opening
        {
            std::cerr << String::ucompose(_("Error opening `%1' for reading.  Exiting."), filename) << std::endl;
            exit(-1);
        }

        char tmp;
        d_fin->read(&tmp,1);
        debug("IS ZIPPED ->" << tmp)                

        if (d_fin->eof())
        {
            std::cerr << String::ucompose(_("Zip file `%1' is too short.  probably broken. Skipping load."), filename) << std::endl;
            d_failed = true;
        }

        if (tmp=='Z') 
        {  
            std::cout <<filename << ": The file is obfuscated, attempting to read it....\n"; 
            d_fin->seekg (0,std::ios::end);
            long length = d_fin->tellg();
            d_fin->seekg (1, std::ios::beg);    

            uLongf destlen1=0;
            (*d_fin) >> destlen1;

            debug(destlen1 << " -- "<< length);

            length--;
            length-=sizeof(uLongf);

            char * buffer = (char*) malloc(length);        
            char * buf1 = (char *) malloc(destlen1);

            d_fin->read(buffer,length);
            d_fin->close();

            uncompress((Bytef*)buf1,&destlen1,(Bytef*)buffer,length);

            free(buffer);
            debug(destlen1 << " -- "<< length);
        
            d_inbuf = new std::istringstream(buf1);

            free(buf1);
            d_in=d_inbuf;
        }
        else 
        {
            //std::cout <<filename <<_(": The file is not obfuscated, attempting to read it....\n"); 
            d_fin->seekg(0, std::ios::beg);
            d_in = d_fin;
        }
    }

    if (mode & std::ios::out)
    {
        d_fout = new std::ofstream(filename.c_str(),
                                   std::ios::out & std::ios::trunc);
        if (!(*d_fout))
        {
            std::cerr << String::ucompose(_("Error opening `%1' for writing.  Exiting."), filename) << std::endl;
            exit(-1);
        }

        d_outbuf = new std::ostringstream();
        d_out = d_outbuf;
    }
}

XML_Helper::XML_Helper(std::ostream* output)
  : d_inbuf(0), d_outbuf(0), d_fout(0), d_fin(0), d_out(0), d_in(0),
    d_last_opened(""), d_version(""), d_failed(false), d_zip(false)
{
  d_out = output;
}

XML_Helper::XML_Helper(std::istream* input)
  : d_inbuf(0), d_outbuf(0), d_fout(0), d_fin(0), d_out(0), d_in(0),
    d_last_opened(""), d_version(""), d_failed(false), d_zip(false)
{
  d_in = input;
}

XML_Helper::~XML_Helper()
{
    if (d_tags.size() != 0)
    // should never happen unless there was an error
        std::cerr << "XML_Helper: dtags not empty!!\n";
    
    debug("Called destructor\n")    
    close();
}

bool XML_Helper::begin(Glib::ustring version)
{
    d_version = version;
    (*d_out) << xml_entity << std::endl;

    return true;
}

bool XML_Helper::openTag(Glib::ustring name)
{
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given\n";
        return false;
    }

    if ((name[0] == 'd') && (name[1] == '_'))
    {
        std::cerr <<name << ": The tag name starts with a \"d\". Not creating tag!\n";
        return false;
    }

    addTabs();

    // append the version strin got the first opened tag
    if (d_tags.empty())
        (*d_out) <<"<" <<name <<" version=\"" <<d_version <<"\">\n";
    else
        (*d_out) <<"<" <<name <<">\n";
        
    d_tags.push_front(name);
    return true;
}

bool XML_Helper::closeTag()
{
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given.\n";
        return false;
    }

    Glib::ustring name = (*d_tags.begin());
    
    //remove tag from list
    d_tags.pop_front();

    addTabs();
    (*d_out) <<"</" <<name <<">\n";

    return true;
}

bool XML_Helper::saveData(Glib::ustring name, const Gdk::RGBA value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr << "XML_Helper: save_data with empty name\n";
        return false;
    }
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given.\n";
        return false;
    }

    addTabs();
    char buf[3];
    guint32 r, g, b;
    r = value.get_red() * 255;
    g = value.get_green() * 255;
    b = value.get_blue() * 255;
    snprintf(buf, sizeof(buf), "%02X", r);
    Glib::ustring red = buf;
    snprintf(buf, sizeof(buf), "%02X", g);
    Glib::ustring green = buf;
    snprintf(buf, sizeof(buf), "%02X", b);
    Glib::ustring blue = buf;

    (*d_out) <<"<" <<name <<">#" <<red <<green<<blue <<"</" <<name <<">\n";
  return true;
}

bool XML_Helper::saveData(Glib::ustring name, Glib::ustring value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr << "XML_Helper: save_data with empty name\n";
        return false;
    }
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given.\n";
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(Glib::ustring name, int value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr << "XML_Helper: save_data with empty name\n";
        return false;
    }
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given.\n";
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(Glib::ustring name, guint32 value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr << "XML_Helper: save_data with empty name\n";
        return false;
    }
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given.\n";
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(Glib::ustring name, bool value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr << "XML_Helper: save_data with empty name\n";
        return false;
    }
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given.\n";
        return false;
    }

    Glib::ustring s;
    s = (value? "true" : "false");

    addTabs();
    (*d_out) <<"<" <<name <<">" <<s <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(Glib::ustring name, double value)
{
    //prepend a "d_" to show that this is a data tag
    name = "d_" + name;

    if (name.empty())
    {
        std::cerr << "XML_Helper: save_data with empty name\n";
        return false;
    }
    if (!d_out)
    {
        std::cerr << "XML_Helper: no output stream given.\n";
        return false;
    }

    addTabs();
    (*d_out) <<"<" <<name <<">" <<value <<"</" <<name <<">\n";
    return true;
}

bool XML_Helper::saveData(Glib::ustring name, unsigned long int value)
{
    return saveData(name, static_cast<guint32>(value));
}

bool XML_Helper::close()
{
    if (d_outbuf)        
    {
        if (d_zip)        
        {
            debug("I zip IT")
            debug("Saving game and obfuscating the Savefile.\n") 
            Glib::ustring tmp = d_outbuf->str();
            tmp+='\0';

            long origlength = tmp.length();
            uLongf ziplength = static_cast<uLongf>(origlength + (12+0.01*origlength));
            char *buf1= (char*) malloc(ziplength);
         
            compress2((Bytef*)buf1, &ziplength, (Bytef*)tmp.c_str(), 
                      (uLong)origlength, 9);

            (*d_fout) << 'Z';
            (*d_fout) << origlength;

            d_fout->write(buf1, ziplength);
            d_fout->flush();

            free(buf1);
            delete d_outbuf;

            d_outbuf = 0;
            debug("destroyed d_outbuf")
        }
        else 
        {
            debug("I do not zip IT")
            debug(_("Saving game without obfuscation.\n")) 

            std::string tmp = d_outbuf->str();
            d_fout->write(tmp.c_str(), tmp.length());
            d_fout->flush();

            delete d_outbuf;
            d_outbuf = 0;
            debug("destroyed d_outbuf")
        }
    }

    if (d_inbuf)        
    {
        delete d_inbuf;
        d_inbuf = 0;
        debug("destroyed d_inbuf")
    }

    if (d_fout)
    {
        d_fout->close();
        delete d_fout;
        d_fout = 0;
    }

    if (d_fin)
    {
        d_fin->close();
        delete d_fin;
        d_fin = 0;
    }

    d_out = 0;
    d_in = 0;
        
    return true;
}

void XML_Helper::addTabs()
{
    for (unsigned int i = d_tags.size(); i > 0; i--)
        (*d_out)<<"\t";
}

//loading
bool XML_Helper::registerTag(Glib::ustring tag, XML_Slot callback)
{
    //register tag as important
    d_callbacks[tag] = callback;

    return true;
}

bool XML_Helper::unregisterTag(Glib::ustring tag)
{
    std::map<Glib::ustring, XML_Slot>::iterator it = d_callbacks.find(tag);

    if (it == d_callbacks.end())
    //item doesn't exist
        return false;
    
    d_callbacks.erase(it);
    return true;
}

bool XML_Helper::getData(Gdk::RGBA & data, Glib::ustring name)
{
    //the data tags are stored with leading "d_", so prepend it here
    name = "d_" + name;

    std::map<Glib::ustring, Glib::ustring>::const_iterator it;

    it = d_data.find(name);
    
    if (it == d_data.end())
    {
        data.set_rgba(0,0,0);
        std::cerr<<String::ucompose(_("Error!  couldn't get Gdk::RGBA value from xml tag `%1'."), name) << std::endl;
        d_failed = true;
        return false;
    }
    
    Glib::ustring value = (*it).second;
    char buf[15];
    int retval = sscanf(value.c_str(), "%s", buf);
    if (retval == -1)
      return false;
    buf[14] = '\0';
    int red = 0, green = 0, blue = 0;
    if (buf[0] == '#')
      {
	char hash;
	//must look like "#00FF33"
	retval = sscanf(buf, "%c%02X%02X%02X", &hash, &red, &green, &blue);
	if (retval != 4)
	  return false;
      }
    else
      {
	//must look like "123 255 000"
	retval = sscanf(value.c_str(), "%d%d%d", &red, &green, &blue);
	if (retval != 3)
	  return false;
	if (red > 255 || red < 0 || green > 255 || green < 0 || 
	    blue > 255 || blue < 0)
	  return false;
      }
    data.set_rgba((float)red / 255.0, (float)green / 255.0,
		   (float)blue / 255.0);
  return true;
}

bool XML_Helper::getData(Glib::ustring& data, Glib::ustring name)
{
    //the data tags are stored with leading "d_", so prepend it here
    name = "d_" + name;

    std::map<Glib::ustring, Glib::ustring>::const_iterator it;

    it = d_data.find(name);
    
    if (it == d_data.end())
    {
        data = "";
        std::cerr<<String::ucompose(_("Error!  couldn't get Glib::ustring value from xml tag `%1'."), name) << std::endl;
        d_failed = true;
        return false;
    }
    
    data = (*it).second;

    return true;
}

bool XML_Helper::getData(bool& data, Glib::ustring name)
{
    //the data tags are stored with leading "d_", so prepend it here
    name = "d_" + name;

    std::map<Glib::ustring, Glib::ustring>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<String::ucompose(_("Error!  couldn't get bool value from xml tag `%1'."), name) << std::endl;
        d_failed = true;
        return false;
    }
    
    if ((*it).second == "true")
    {
        data = true;
        return true;
    }

    if ((*it).second == "false")
    {
        data = false;
        return true;
    }
    
    return false;
}

bool XML_Helper::getData(int& data, Glib::ustring name)
{
    //the data tags are stored with leading "d_", so prepend it here
    name = "d_" + name;

    std::map<Glib::ustring, Glib::ustring>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<String::ucompose(_("Error!  couldn't get int value from xml tag `%1'."), name) << std::endl;
        d_failed = true;
        return false;
    }
    
    data = atoi((*it).second.c_str());
    return true;
}

bool XML_Helper::getData(guint32& data, Glib::ustring name)
{
    //the data tags are stored with leading "d_", so prepend it here
    name = "d_" + name;

    std::map<Glib::ustring, Glib::ustring>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<String::ucompose(_("Error!  couldn't get guint32 value from xml tag `%1'."), name) << std::endl;
        d_failed = true;
        return false;
    }
    
    data = static_cast<guint32>(atoi((*it).second.c_str()));
    return true;

    
}

bool XML_Helper::getData(double& data, Glib::ustring name)
{
    //the data tags are stored with leading "d_", so prepend it here
    name = "d_" + name;

    std::map<Glib::ustring, Glib::ustring>::const_iterator it;
    it = d_data.find(name);

    if (it == d_data.end())
    {
        std::cerr<<String::ucompose(_("Error!  couldn't get double value from xml tag `%1'."), name) << std::endl;
        d_failed = true;
        return false;
    }

    data = strtod((*it).second.c_str(), 0);
    return true;
}

bool XML_Helper::parse()
{
    if (!d_in || d_failed)
        return false;
        //what's the use of parsing no or incorrect input?

    d_parser = XML_ParserCreate("utf-8");

    //set handlers and data
    XML_SetElementHandler(d_parser, start_handler, end_handler);
    XML_SetCharacterDataHandler(d_parser, character_handler);
    XML_SetUserData(d_parser, static_cast<void*>(this));

    while (!d_in->eof() && !d_failed)
    {
        void * buffer = XML_GetBuffer(d_parser,1024);
        d_in->read((char*)buffer, 1024);
          int bytesread = d_in->gcount();
        //int bytesread = d_in->getline(buffer, 1000);
        bool my_eof = d_in->eof();

        if (!XML_ParseBuffer(d_parser, bytesread, my_eof))
        //error parsing
        {
            std::cerr << String::ucompose(_("Error parsing xml document.  Line %1\n%2"), XML_GetCurrentLineNumber(d_parser), XML_ErrorString(XML_GetErrorCode(d_parser))) << std::endl;
            d_failed = true;
        }
    }

    XML_ParserFree(d_parser);

    return (!d_failed);
}

//beginning with here is only internal stuff. Continue reading only if you are
//interested in the xml parsing. :)

/* Parsing works like this: We have three expat callback functions,
 * startelementhandler, endelementhandler and cdatahandler.
 * The startelement handler just calls XML_Helper::tag_open (an object is passed
 * as data), the cdata element handler just sums up the cdata, and the end
 * element handler calls XML_Helper::tag_close, giving it also the final cdata
 * string (the string between opened tag and closed tag) to the XML_Helper.
 * Since data is always stored like "<mydata>data_value</mydata>", having
 * a startelementhandler encounter a non-null summed up cdata string is a
 * serious error and results in a fail.
 *
 * Now the XML_Helper functions: 
 * tag_open looks if another important tag has already been opened last (and not
 * called back). If so, it assumes that all important data has already been
 * stored and calls the callback for the former tag. If not, it just goes on.
 * last_opened is always set to the last opened tag marked as important.
 * If tag_close is called, it is mostly for data. If cdata is != 0 it is some
 * saved data. If the last_opened tag is the same as the closed tag (we disallow
 * and thus ignore constructions like "<mytag> <mytag> </mytag> </mytag>" here,
 * they are IMO pointless), we suppose that the callback has not been called yet
 * and do it now. If not, then there has been another important tag on the way
 * which has led tag_open to already call the callback.
 */

bool XML_Helper::tag_open(Glib::ustring tag, Glib::ustring version, Glib::ustring lang)
{
    if (d_failed)
        return false;
        
    //first of all, register the tag as opened
    d_tags.push_front(tag);

    if (version != "")
        d_version = version;

    //look if the tag starts with "d_". If so, it is a data tag without anything
    //important in between
    if ((tag[0] == 'd') && (tag[1] == '_'))
      {
	d_lang[tag] = lang;
	return true;
      }
    
    //first of all, look if another important tag has already been opened
    //and call the appropriate callback if so
    std::list<Glib::ustring>::iterator ls_it;
    ls_it = d_tags.begin();
    ls_it++;

    if ((ls_it != d_tags.end()) && (d_last_opened == (*ls_it)))
    {
        std::map<Glib::ustring, XML_Slot>::iterator it;
        it = d_callbacks.find(*ls_it);

        
        if (it != d_callbacks.end())
        {
            //make the callback (yes that is the syntax, overloaded "()")
            bool retval = (it->second)(*ls_it, this);
            if (retval == false)
            {
                std::cerr << String::ucompose(_("%1: Callback for xml tag returned false.  Stop parsing document."), (*ls_it)) << std::endl;
                error = true;
                d_failed = true;
            }
        }

        //clear d_data (we are just setting up a new tag)
        d_data.clear();
        d_lang.clear();
    }

    d_last_opened = tag;

    return true;
}

bool XML_Helper::lang_check(Glib::ustring lang)
{
  static char *envlang = getenv("LANG");
  if (envlang == NULL)
    envlang = getenv("LC_ALL");
  if (envlang == NULL)
    envlang = getenv("LC_CTYPE");
  if (lang == "")
    return true;
  if (envlang == NULL)
    return false;
  if (lang == envlang)
    return true;
  //try harder
  char *first_underscore = strchr (envlang, '_');
  if (first_underscore)
    {
      if (strncmp (lang.c_str(), envlang, first_underscore - envlang) == 0)
	return true;
    }
  return false;
}

bool XML_Helper::tag_close(Glib::ustring tag, Glib::ustring cdata)
{
    if (d_failed)
        return false;

    //remove tag entry, there is nothing more to be done
    d_tags.pop_front();
    
    if ((tag[0] == 'd') && (tag[1] == '_'))
    {
        // save the data (we close a data tag)
	if (lang_check(d_lang[tag]))
	  d_data[tag] = cdata;
        return true;    //data tags end here with their execution
    }

    if ((d_last_opened == tag))
    //callback hasn't been called yet
    {
        std::map<Glib::ustring, XML_Slot>::iterator it;
        it = d_callbacks.find(tag);
        
        if (it != d_callbacks.end())
        {
            //make the callback (yes that is the syntax, overloaded "()")
            bool retval = it->second(tag, this);
            
            if (retval == false)
            {
                std::cerr << String::ucompose(_("%1: Callback for xml tag returned false.  Stop parsing document."), tag) << std::endl;
                error = true;
                d_failed = true;
            }
        }
    }

    //clear d_data (we are just setting up a new tag)
    d_data.clear();
    d_lang.clear();
    
    return true;
}

//these functions are the expat callbacks. Their main purpose is to set up
//the parametres and call the tag_open and tag_close functions in XML_Helper.

//the start handler, call open_tag and do misc init stuff
void start_handler(void* udata, const XML_Char* name, const XML_Char** atts)
{
    if (error)
        return;
    
    XML_Helper* helper = static_cast<XML_Helper*>(udata);
    Glib::ustring version = "";
    Glib::ustring lang = "";

    //the only attribute we know and handle are version strings
    if ((atts[0] != 0) && (Glib::ustring(atts[0]) == "version"))
        version = Glib::ustring(atts[1]);

    if ((atts[0] != 0) && (Glib::ustring(atts[0]) == "xml:lang"))
        lang = Glib::ustring(atts[1]);

    my_cdata = "";

    error = !helper->tag_open(Glib::ustring(name), version, lang);
}

//the cdata handler, just sums up the string  s
void character_handler(void* udata, const XML_Char* s, int len)
{
    if (error)
        return;

    XML_Char buffer[len+4];
    
    memset (buffer, 0, sizeof (buffer));
    memcpy (buffer, s, len);
    //strncpy(buffer, s, len);
    //buffer[len] = '\0';

    //now add the string to the other one
    //my_cdata += Glib::ustring(buffer);
    my_cdata += Glib::ustring((const char *) buffer);
}

//the end handler: call tag_close and dosome cleanup
void end_handler(void* udata, const XML_Char* name)
{
    if (error)
        return;
    
    XML_Helper* helper = static_cast<XML_Helper*>(udata);

    error = !helper->tag_close(Glib::ustring(name), my_cdata);

    my_cdata = "";
}

Glib::ustring XML_Helper::get_top_tag(Glib::ustring filename, bool zip)
{
  char buffer[1024];
  XML_Helper in(filename, std::ios::in, zip);
  while (in.d_in->eof() == false)
    {
      in.d_in->getline(buffer, sizeof buffer);
      Glib::ustring line(buffer);
      if (line.find("<?xml version=\"1.0\"") == 0)
        continue;
      size_t start = line.find('<');
      if (start == Glib::ustring::npos)
        continue;
      size_t finish = line.find(" version=", start + 1);
      if (finish == Glib::ustring::npos)
        continue;
      in.close();
      return line.substr(start + 1, finish - start - 1);
    }
  in.close();
  return "";
}

bool XML_Helper::rewrite_version(Glib::ustring filename, Glib::ustring tag, Glib::ustring new_version, bool zip)
{
  Glib::ustring match = "<" + tag + " version=\"";
  bool found = false;
  char buffer[1024];
  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper in(filename, std::ios::in, zip);
  XML_Helper out(tmpfile, std::ios::out, zip);
  while (in.d_in->eof() == false)
    {
      in.d_in->getline(buffer, sizeof buffer);
      Glib::ustring line(buffer);
      if (line.compare(0, match.length(), match) == 0 && found == false)
        {
          found = true;
          Glib::ustring upgraded_line = match + new_version + "\">";
          out.d_out->write(upgraded_line.c_str(), upgraded_line.length());
          (*out.d_out) << std::endl;
        }
      else
        {
          int len = in.d_in->gcount();
          size_t pos = line.rfind("\r\n");
          if (pos == Glib::ustring::npos)
            {
              pos = line.rfind('\n');
              if (pos != Glib::ustring::npos)
                len--;
            }
          else
            len-=2;
          if (len)
            {
              if (buffer[len-1] == '\0')
                len--;
              out.d_out->write(buffer, len);
            }
          (*out.d_out) << std::endl;
        }
    }
  out.close();
  in.close();
  File::erase(filename);
  rename(tmpfile.c_str(), filename.c_str());
  return found;
}
