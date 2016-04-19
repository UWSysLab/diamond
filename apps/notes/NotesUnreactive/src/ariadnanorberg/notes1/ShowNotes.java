package ariadnanorberg.notes1;

import java.util.ArrayList;
import java.util.List;

import com.parse.FindCallback;
import com.parse.GetCallback;
import com.parse.ParseException;
import com.parse.ParseObject;
import com.parse.ParseQuery;
import com.parse.ParseUser;
import com.parse.RequestPasswordResetCallback;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;
import android.widget.Toolbar;

public class ShowNotes extends ListActivity {
	private List<Note> posts;
	private Toolbar toolbar;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		Intent intent = getIntent();
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shownotes);
		ParseUser currentUser = ParseUser.getCurrentUser();
		if (currentUser == null) {
			loadLoginView();
		}
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
	                
	                case R.id.action_logout: {
	                	ParseUser.logOut();
	                	loadLoginView();
	                	break;
	                }
                }
                return false;        
            }
		});
	}
	
	private void refreshNotesList() {
		final long startTime = System.currentTimeMillis();
		// refreshes list of notes to current state
		ParseQuery<ParseObject> query = ParseQuery.getQuery("Post");
		query.whereEqualTo("author", ParseUser.getCurrentUser());
		setProgressBarIndeterminateVisibility(true);
		query.findInBackground(new FindCallback<ParseObject>() {
			
			@Override
			public void done(List<ParseObject> postList, ParseException e) {
				setProgressBarIndeterminateVisibility(false);
				if (e == null) {
					//if there are results, update the list of notes and notify adapter
					posts.clear();
					for (ParseObject post: postList) {
						Note note = new Note(post.getObjectId(), post.getString("title"), post.getString("content"));
						posts.add(note);
					}
					((ArrayAdapter<Note>) getListAdapter()).notifyDataSetChanged();
				} else {
					Log.d(getClass().getSimpleName(), "Error: " + e.getMessage());
					System.err.println("An IO error was caught :" + e.getMessage());
					System.err.println(e);
					e.printStackTrace();
				}
			}
		});
		final long endTime = System.currentTimeMillis();
		// prints execution time to make parse query and display the notes
		System.out.println("Total execution time: " + (endTime - startTime) + "ms");
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
	    Note note = posts.get(position);
	    Intent intent = new Intent(this, EditNoteActivity.class);
	    intent.putExtra("noteId", note.getId());
	    intent.putExtra("noteTitle", note.getTitle());
	    intent.putExtra("noteContent", note.getContent());
	    startActivity(intent);
	}
	
	private void loadLoginView() {
		Intent intent = new Intent(this, LoginActivity.class);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK); // clears stack history and brings loginactivity to front
		intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
		startActivity(intent);
	}
}