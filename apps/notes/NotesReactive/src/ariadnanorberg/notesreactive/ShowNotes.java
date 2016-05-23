package ariadnanorberg.notesreactive;

import java.util.ArrayList;
import java.util.List;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toolbar;
import ariadnanorberg.notesreactive.Note;

public class ShowNotes extends ListActivity {
	private List<Note> posts;
	private Toolbar toolbar;
	private ArrayList<String> titles;
	private ArrayList<String> contents;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shownotes);
		
		posts = new ArrayList<Note>();
		ArrayAdapter<Note> adapter = new ArrayAdapter<Note>(this, R.layout.list_item_layout, posts);
		setListAdapter(adapter);
		
		Intent intent = this.getIntent();
		if (intent.getExtras() != null) {
				titles = intent.getStringArrayListExtra("titles");
		        contents = intent.getStringArrayListExtra("contents");
		}
		refreshNotesList();
		
		toolbar = (Toolbar)findViewById(R.id.toolbar1);
		toolbar.inflateMenu(R.menu.main);
		
		toolbar.setOnMenuItemClickListener(new Toolbar.OnMenuItemClickListener() {
            @Override
            public boolean onMenuItemClick(MenuItem menuItem) {

                switch (menuItem.getItemId()){
	                case R.id.action_refresh: {
	                    refreshNotesList();
	                    break;
	                }
	
	                case R.id.action_new: {
	                    Intent intent = new Intent(ShowNotes.this, EditNoteActivity.class);
	                    startActivity(intent);
	                    break;
	                }
	                
	                /*case R.id.action_logout: {
	                	// logout
	                	loadLoginView();
	                	break;
	                }*/
                }
                return false;        
            }
		});
	}
	
	private void refreshNotesList() {
		//final long startTime = System.currentTimeMillis();
		//final long endTime = System.currentTimeMillis();
		// prints execution time to make parse query and display the notes
		if (titles != null) {
			posts.clear();
			for (int i = 0; i < titles.size(); i++) {	
				Note note = new Note(titles.get(i), contents.get(i));
				posts.add(note);
				//titles.add(note.getTitle());
		    	//contents.add(note.getContent());
			}
			//System.out.println("Total execution time: " + (endTime - startTime) + "ms");
		} else {
	    	Intent redirect = new Intent(this, EditNoteActivity.class);
			startActivity(redirect);
	    }
		System.out.println(posts.toString());
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
	    Note note = posts.get(position);
	    Intent intent = new Intent(this, EditNoteActivity.class);
	    intent.putExtra("noteTitle", note.getTitle());
	    intent.putExtra("noteContent", note.getContent());
	    //intent.putStringArrayListExtra("titles", titles);
	    //intent.putStringArrayListExtra("contents", contents);
	    startActivity(intent);
	}
	
	/*private void loadLoginView() {
		Intent intent = new Intent(this, LoginActivity.class);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK); // clears stack history and brings loginactivity to front
		intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
		startActivity(intent);
	} */
}