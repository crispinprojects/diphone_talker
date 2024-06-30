/***************************************************************************
 *   Author Alan Crispin                                                   *
 *   crispinalan@gmail.com                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation.                                         *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <gtk/gtk.h>
#include "dictionary.h"
#include "diphone.h"

static GMutex lock;

static void callbk_preferences(GSimpleAction* action, GVariant *parameter,gpointer user_data);

//static void callbk_open_file(GtkButton *button, gpointer  user_data);
//static void callbk_clear(GtkButton *button, gpointer  user_data);
static void callbk_open_file(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static void callbk_quit(GSimpleAction* action,G_GNUC_UNUSED GVariant *parameter, gpointer user_data);
static void callbk_about(GSimpleAction* action, GVariant *parameter, gpointer user_data);
static GMenu *create_menu(const GtkApplication *app);

static gpointer thread_playraw(gpointer user_data);
static int m_talk_rate=7000;

//--------------------------------------------------------------------
// Removers (unwanted characters}
//--------------------------------------------------------------------

static char *remove_commas(const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new("");
	p = text;
	while (*p)
	{
		gunichar cp = g_utf8_get_char(p);
		if (cp != ',')
		{ 
			g_string_append_unichar(str, *p);
		} // if
		++p;
	}
	return g_string_free(str, FALSE);
}

static char *remove_fullstop(const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new("");
	p = text;
	while (*p)
	{
		gunichar cp = g_utf8_get_char(p);
		if (cp != '.')
		{ 
			g_string_append_unichar(str, *p);
		} // if
		++p;
	}
	return g_string_free(str, FALSE);
}
static char* remove_semicolons (const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != ';' ){ 
	g_string_append_unichar (str, *p);
	}//if
	++p;
	}
	return g_string_free (str, FALSE);
}

static char* remove_question_marks (const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != '?' ){ 
	g_string_append_unichar (str, *p);
	}//if
	++p;
	}
	return g_string_free (str, FALSE);
}

static char* remove_explanation_marks (const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != '!' ){ 
	g_string_append_unichar (str, *p);
	}//if
	++p;
	}
	return g_string_free (str, FALSE);
}

static char* remove_punctuations(const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != '\'' ){ //remove all apostrophes
	g_string_append_unichar (str, *p);
	}//if
	++p;
	}
	return g_string_free (str, FALSE);
}

static char* replace_hypens(const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	gint i=0;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != '-' ){ //append
	g_string_append_unichar (str, *p);
	}//if
	if ( cp == '-' ){ //replace			
	g_string_insert_unichar (str,i,' ');
	}//if	
	++p;
	++i;
	}
	return g_string_free (str, FALSE);
}

static char* replace_newlines(const char *text)
{
	GString *str;
	const char *p;
	str = g_string_new ("");
	p = text;
	gint i=0;
	while (*p)
	{
	gunichar cp = g_utf8_get_char(p);
	if ( cp != '\n' ){ //append
	g_string_append_unichar (str, *p);
	}//if
	if ( cp == '\n' ){ //replace			
	g_string_insert_unichar (str,i,' ');
	}//if	
	++p;
	++i;
	}
	return g_string_free (str, FALSE);
}

//--------------------------------------------------------------------
//play (thread it)
//--------------------------------------------------------------------

//---------------------------------------------------------------
// play wav file thread
//---------------------------------------------------------------
static gpointer thread_playraw(gpointer user_data)
{   
    //gchar *wav_path =user_data;   
    //gchar * command_str ="aplay";    
    //gchar *m_sample_rate_str = g_strdup_printf("%i", m_talk_sample_rate); 
    //gchar *sample_rate_str ="-r ";    
    //sample_rate_str= g_strconcat(sample_rate_str,m_sample_rate_str, NULL); 
   	//char input[50];		
	//strcpy(input, wav_path);	  
    //command_str =g_strconcat(command_str," ",sample_rate_str, " ", input, NULL); 
    //system(command_str); 
    
    gchar *raw_file =user_data;       
    gchar *m_sample_rate_str = g_strdup_printf("%i", m_talk_rate); 
    gchar *sample_rate_str ="-r ";    
    sample_rate_str= g_strconcat(sample_rate_str,m_sample_rate_str, NULL);     
    //gchar * command_str ="aplay -c 1 -f S16_LE";
    gchar * command_str ="aplay -c 1 -f U8";
    command_str =g_strconcat(command_str," ",sample_rate_str, " ", raw_file, NULL); 
    system(command_str); 
           
    g_mutex_unlock (&lock); //thread mutex unlock 
    return NULL; 
}


//void  playraw(gpointer user_data)
//{   
    //gchar *raw_file =user_data;       
    //gchar *m_sample_rate_str = g_strdup_printf("%i", m_talk_rate); 
    //gchar *sample_rate_str ="-r ";    
    //sample_rate_str= g_strconcat(sample_rate_str,m_sample_rate_str, NULL);     
    ////gchar * command_str ="aplay -c 1 -f S16_LE";
    //gchar * command_str ="aplay -c 1 -f U8";
    //command_str =g_strconcat(command_str," ",sample_rate_str, " ", raw_file, NULL); 
    //system(command_str); 
//}

//--------------------------------------------------------------------
//concatenate
//--------------------------------------------------------------------

unsigned char *rawcat(unsigned char *arrys[], unsigned int arry_size[], int arry_count) {
		
	
	if (arry_count<2) return NULL;	
	
	unsigned int  total_samples=0;
	for (int c = 0; c < arry_count; c++) 
	{  
    unsigned int count =arry_size[c]; 
    total_samples=total_samples+count;	
    }        
	unsigned char *data = (unsigned char*)malloc(total_samples * sizeof(unsigned char));
	
	unsigned int offset=0;
	for(int k=0; k<arry_count; k++)
	{
		//loop through each arry	
		for(int i = 0; i < arry_size[k]; i++) 
		{		
		data[i+offset]=arrys[k][i];
		}		
		offset =offset+arry_size[k];
	}//k kount
	return data;
}

unsigned int get_merge_size(unsigned int sizes_arry[], int arry_size){
	
	unsigned int total_samples=0;
	for (int i = 0; i < arry_size; i++) 
	{  
    unsigned int count =sizes_arry[i]; 
    total_samples=total_samples+count;	
    }
	return total_samples;
}

//--------------------------------------------------------------------
// speak
//--------------------------------------------------------------------

static void callbk_speak(GSimpleAction * action,G_GNUC_UNUSED GVariant *parameter, gpointer user_data)
{
	
	GtkWindow *window =user_data;
	
	GtkWidget *textview = g_object_get_data(G_OBJECT(window), "window-textview-key"); //object-key association 
	
	GtkTextBuffer *textview_buffer;
	//GtkTextIter range_start, range_end;
	GtkTextIter start_iter, end_iter;
	
	textview_buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));	
	
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(textview_buffer), &start_iter);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(textview_buffer), &end_iter);
			
	gchar* tv_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(textview_buffer),&start_iter,&end_iter,FALSE);
	
	const char *text =g_ascii_strdown(tv_text, -1);	//lower	
	
	//very simple input text processing
	text= replace_newlines(text);
	text = remove_semicolons(text);
	text = remove_commas(text);
	text = remove_fullstop(text);
	text = remove_question_marks(text);	
	text= remove_explanation_marks(text);
	text =remove_punctuations(text);
	text = replace_hypens(text); //replace hypens with space
	
	g_print("processed text = %s\n", text);
	
	gchar** words;	
	GList *diphone_list = NULL;			
	words = g_strsplit (text, " ", 0); //split on space	
	int j=0; //count			   
	while(words[j] != NULL)
	{	
	g_print("word =%s\n",words[j]);
	diphone_list =g_list_concat(diphone_list,word_to_diphones(words[j]));
	//diphone_list =g_list_concat(diphone_list, word_to_diphones("pau"));
	j++;
	} //while loop words
		
	gpointer diphone_list_pointer;
	gchar* diphone_str;		
	gint diphone_number  =g_list_length(diphone_list);
	g_print("diphone_number = %i\n", diphone_number);
	//create diphone array using list length
	unsigned char *diphone_arrays[diphone_number]; 
	unsigned int diphone_arrays_sizes[diphone_number];
		
	//load diphone arrays
	for(int i=0; i <diphone_number; i++)
	{
		diphone_list_pointer=g_list_nth_data(diphone_list,i);
		diphone_str=(gchar *)diphone_list_pointer;					
		g_print("diphone = %s\n",diphone_str);		
		diphone_arrays[i] = load_diphone_arry(diphone_str);
		g_print("diphone length = %i\n",load_diphone_len(diphone_str));
		diphone_arrays_sizes[i]=load_diphone_len(diphone_str);		
	}	
	
	//concatenate using raw cat
	unsigned char *data = rawcat(diphone_arrays, diphone_arrays_sizes, diphone_number);	
	unsigned int data_len = get_merge_size(diphone_arrays_sizes,diphone_number);	
    
    //gchar* raw_file ="/tmp/talkout.raw";
	//FILE* f = fopen(raw_file, "w");
    //fwrite(data, data_len, 1, f);
    //fclose(f);  
    //playraw(raw_file);
	
	//play audio in a thread
		
	gchar* raw_file ="/tmp/talkout.raw";
	FILE* f = fopen(raw_file, "w");
    fwrite(data, data_len, 1, f);
    fclose(f); 
    
	GThread *thread_audio; 
	g_mutex_lock (&lock);
    thread_audio = g_thread_new(NULL, thread_playraw, raw_file);  
	g_thread_unref (thread_audio);
	
	//clean up 
	g_list_free(diphone_list);	
	free(data);	//prevent memory leak
	
}

//--------------------------------------------------------------------
// callbk openfile
//--------------------------------------------------------------------
//static void callbk_clear(GtkButton *button, gpointer  user_data)
//{
		
		//GtkWindow *window =user_data;
		////g_print("Clear called\n");				
		//GtkWidget *textview = g_object_get_data(G_OBJECT(window), "window-textview-key"); //object-key association 
		//GtkTextBuffer *textbuffer;  	 
		//textbuffer = gtk_text_buffer_new (NULL);   
		//gchar *output_str =""; 
		////gtk_text_buffer_set_text(textbuffer, "", 0);
		//gtk_text_buffer_set_text (textbuffer, output_str, -1);	//CLEAR
		//gtk_text_view_set_buffer(GTK_TEXT_VIEW(textview), textbuffer); 
//}

//static void callbk_response(GtkDialog *dialog, gpointer  user_data)
static void callbk_response (GtkDialog *dialog, int response)
{   
	GtkWidget *textview = g_object_get_data(G_OBJECT(dialog), "dialog-textview-key"); 
	//GtkWidget *textview = g_object_get_data(G_OBJECT(window), "window-textview-key"); //object-key association 
	 
	GtkTextBuffer *textbuffer;
	gchar *contents;
	gsize length;
	
	if (response == GTK_RESPONSE_ACCEPT)
	{
	g_autoptr (GFile) file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
	
	if (! g_file_load_contents (file, NULL, &contents, &length, NULL, NULL)) return;
	
	gchar* filename = g_file_get_path(file);
	g_print("filesname = %s\n",filename);
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
	gtk_text_buffer_set_text (textbuffer, contents, length);
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(textview),GTK_TEXT_BUFFER(textbuffer)); 
	g_free (contents); 
	}
	
	gtk_window_destroy (GTK_WINDOW (dialog));
}

static void callbk_open_file(GSimpleAction* action, GVariant *parameter, gpointer user_data)
{
	g_print("openfile\n");
	
	GtkWindow *window =user_data;   
	
	GtkWidget *textview = g_object_get_data(G_OBJECT(window), "window-textview-key"); //object-key association 
	    
    GtkWidget *dialog;
    
    GtkFileChooserAction file_action = GTK_FILE_CHOOSER_ACTION_OPEN;

	//being depreciated from GTK 4.10 
    dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(window), file_action, ("_Cancel"), GTK_RESPONSE_CANCEL,
                                          ("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show (dialog);
    g_object_set_data(G_OBJECT(dialog), "dialog-textview-key",textview); 
    
    g_signal_connect (dialog, "response", G_CALLBACK (callbk_response),  NULL);
	 
}

//--------------------------------------------------------------------
// callbk preferences
//--------------------------------------------------------------------
static void callbk_set_preferences(GtkButton *button, gpointer  user_data)
{
	
	GtkWidget *dialog = user_data; //should user data be the dialog
	//GtkWidget *dialog = g_object_get_data(G_OBJECT(button), "dialog-key");
	
	GtkWidget *spin_button_sample_rate = g_object_get_data(G_OBJECT(button), "spin-sample-rate-key");
	m_talk_rate = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_button_sample_rate));
	
	gtk_window_destroy(GTK_WINDOW(dialog));
	
}

static void callbk_preferences(GSimpleAction* action, GVariant *parameter,gpointer user_data)
{	
	//g_print("Preferences");
	GtkWidget *window =user_data;
	
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *button_set;
	
	GtkWidget *label_sample_rate;
	GtkWidget *spin_button_sample_rate;
	GtkWidget *box_sample_rate;
	
	
	dialog =gtk_window_new(); //gtk_dialog_new_with_buttons to be deprecated gtk4.10

	gtk_window_set_title (GTK_WINDOW (dialog), "Preferences");
	gtk_window_set_default_size(GTK_WINDOW(dialog),350,100);

	box =gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	gtk_window_set_child (GTK_WINDOW (dialog), box);

	button_set = gtk_button_new_with_label ("Set Preferences");
	g_signal_connect (button_set, "clicked", G_CALLBACK (callbk_set_preferences), dialog);
	
	
	//sample rate
	GtkAdjustment *adjustment_sample_rate;
	// value,lower,upper,step_increment,page_increment,page_size
	adjustment_sample_rate = gtk_adjustment_new(7000.00, 4000.00, 16000.00, 1000.0, 1000.0, 0.0);
	// start time spin
	label_sample_rate = gtk_label_new("Voice Sampling Rate ");
	spin_button_sample_rate = gtk_spin_button_new(adjustment_sample_rate, 500, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button_sample_rate), m_talk_rate);
	box_sample_rate = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_box_append(GTK_BOX(box_sample_rate), label_sample_rate);
	gtk_box_append(GTK_BOX(box_sample_rate), spin_button_sample_rate);

	//data setters
	//g_object_set_data(G_OBJECT(button_set), "dialog-key",dialog);
	g_object_set_data(G_OBJECT(button_set), "spin-sample-rate-key", spin_button_sample_rate);
	
	gtk_box_append(GTK_BOX(box), box_sample_rate);	
	gtk_box_append(GTK_BOX(box), button_set);
	gtk_window_present (GTK_WINDOW (dialog));	
	
	
}

//--------------------------------------------------------------------
// callbk quit
//--------------------------------------------------------------------

static void callbk_quit(GSimpleAction * action,	G_GNUC_UNUSED GVariant *parameter, gpointer user_data)
{
	g_application_quit(G_APPLICATION(user_data));		
}

//--------------------------------------------------------------------
// callbk about
//--------------------------------------------------------------------
static void callbk_about(GSimpleAction* action, GVariant *parameter, gpointer user_data)
{
	GtkWidget *window = user_data;
	const gchar *authors[] = {"Alan Crispin", NULL};
	GtkWidget *about_dialog;
	about_dialog = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(about_dialog),GTK_WINDOW(window));
	gtk_widget_set_size_request(about_dialog, 200,200);
    gtk_window_set_modal(GTK_WINDOW(about_dialog),TRUE);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Talker");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dialog), "0.1.5");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog),"Copyright © 2024");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),"Diphone Speech Synthesizer");
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_LGPL_2_1);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),"https://github.com/crispinprojects/");
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog),"Project Website");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), "x-office-calendar");
	gtk_widget_set_visible (about_dialog, TRUE);	
}

//--------------------------------------------------------------------
// menu
//--------------------------------------------------------------------

static GMenu *create_menu(const GtkApplication *app) {
	
		
	GMenu *menu;
    GMenu *file_menu;
    GMenu *speak_menu;  
    GMenu *help_menu;
    GMenuItem *item;

	menu =g_menu_new();
	file_menu =g_menu_new();
	speak_menu =g_menu_new();
	help_menu =g_menu_new();
	
	//File items	
	
	item =g_menu_item_new("Open", "app.open"); //open file action
	g_menu_append_item(file_menu,item);
	g_object_unref(item);
	
	item =g_menu_item_new("Quit", "app.quit"); //quit action
	g_menu_append_item(file_menu,item);
	g_object_unref(item);
	
	//Speak item	
	item =g_menu_item_new("Speak", "app.speak"); //speak action
	g_menu_append_item(speak_menu,item);
	g_object_unref(item);
	
	//Preferences item	
	item =g_menu_item_new("Preferences", "app.preferences"); //preferences action
	g_menu_append_item(speak_menu,item);
	g_object_unref(item);
			
	//Help item	
	item =g_menu_item_new("About", "app.about"); //about action
	g_menu_append_item(help_menu,item);
	g_object_unref(item);
	
	g_menu_append_submenu(menu, "File", G_MENU_MODEL(file_menu));
    g_object_unref(file_menu);  
    
    g_menu_append_submenu(menu, "Speak", G_MENU_MODEL(speak_menu));
    g_object_unref(speak_menu);  
   
    g_menu_append_submenu(menu, "Help", G_MENU_MODEL(help_menu));
    g_object_unref(help_menu);
    
    return menu;
}


//--------------------------------------------------------------------
// activate
//--------------------------------------------------------------------

static void activate (GtkApplication* app, gpointer user_data)
{
	GtkWidget *window;			
		
	GtkWidget *textview; 
	GtkTextBuffer *textbuffer;
	GtkWidget *scrolledwindow;
	
	GMenu *menu;	
		
	//setup window
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Speech Synthesizer");
	gtk_window_set_default_size (GTK_WINDOW (window),400,200);
	
	//use a textview
	gchar* text ="hello world";	
	textview = gtk_text_view_new ();
	textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
	gtk_text_buffer_set_text (textbuffer, text, -1);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD_CHAR);
	
	
	//actions
	GSimpleAction *quit_action;	
	quit_action=g_simple_action_new("quit",NULL); //app.quit
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(quit_action)); //make visible	
	g_signal_connect(quit_action, "activate",  G_CALLBACK(callbk_quit), app);
	
	GSimpleAction *open_action;	
	open_action=g_simple_action_new("open",NULL); //app.open
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(open_action)); //make visible	
	g_signal_connect(open_action, "activate",  G_CALLBACK(callbk_open_file), window);
	
	GSimpleAction *speak_action;	
	speak_action=g_simple_action_new("speak",NULL); //app.speak
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(speak_action)); //make visible	
	g_signal_connect(speak_action, "activate",  G_CALLBACK(callbk_speak), window);
			
	GSimpleAction *preferences_action;
	preferences_action=g_simple_action_new("preferences",NULL); //app.preferences
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(preferences_action)); //make visible
	g_signal_connect(preferences_action, "activate",  G_CALLBACK(callbk_preferences), window);
	
	GSimpleAction *about_action;
	about_action=g_simple_action_new("about",NULL); //app.about
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(about_action)); //make visible
	g_signal_connect(about_action, "activate",  G_CALLBACK(callbk_about), window);
	
	//menu
	menu=create_menu(app);	
	gtk_application_set_menubar (app,G_MENU_MODEL(menu));
    gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(window), TRUE);
	
	
	g_object_set_data(G_OBJECT(window), "window-textview-key",textview); 		
    gtk_window_set_child (GTK_WINDOW (window), textview);
	
	gtk_window_present(GTK_WINDOW (window));    //use present not show with gtk4	
}

//--------------------------------------------------------------------
// main
//--------------------------------------------------------------------

int main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.diphonetalker", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
