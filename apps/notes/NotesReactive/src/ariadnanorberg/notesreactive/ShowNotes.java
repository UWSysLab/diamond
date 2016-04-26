package ariadnanorberg.notesreactive;

import java.util.ArrayList;
import java.util.List;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toolbar;
import ariadnanorberg.notesreactive.Note;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.ReactiveManager;

public class ShowNotes extends ListActivity {
	private List<Note> posts;
	private Toolbar toolbar;
	private Diamond.DStringList notesList;
	private ArrayList<String> titles;
	private ArrayList<String> contents;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shownotes);
		
		Diamond.DiamondInit("128.208.6.85", "12444");
		ReactiveManager.StartManager();
		ReactiveManager.RegisterLogger(new ReactiveManager.Logger() {
			public void onLog(String message) {
				Log.i("NotesReactive", message);
			}
		});
		
		notesList = new Diamond.DStringList("notesreactive:noteslist");
		posts = new ArrayList<Note>();
		ArrayAdapter<Note> adapter = new ArrayAdapter<Note>(this, R.layout.list_item_layout, posts);
		setListAdapter(adapter);
		
		toolbar = (Toolbar)findViewById(R.id.toolbar1);
		toolbar.inflateMenu(R.menu.main);
		
		refreshNotesList();
		
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
		final long startTime = System.currentTimeMillis();
		final long endTime = System.currentTimeMillis();
		// prints execution time to make parse query and display the notes
		posts.clear();
		for (int i = 0; i < notesList.Size(); i++) {	
			Note note = new Note(notesList.Value(i).split(System.getProperty("line.separator"))[0], notesList.Value(i).split(System.getProperty("line.separator"))[1]);
			posts.add(note);
			titles.add(note.getTitle());
	    	contents.add(note.getContent());
		}
		System.out.println("Total execution time: " + (endTime - startTime) + "ms");
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
	    Note note = posts.get(position);
	    Intent intent = new Intent(this, EditNoteActivity.class);
	    intent.putExtra("noteTitle", note.getTitle());
	    intent.putExtra("noteContent", note.getContent());
	    intent.putStringArrayListExtra("titles", titles);
	    intent.putStringArrayListExtra("contents", contents);
	    startActivity(intent);
	}
	
	/*private void loadLoginView() {
		Intent intent = new Intent(this, LoginActivity.class);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK); // clears stack history and brings loginactivity to front
		intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
		startActivity(intent);
	} */
}