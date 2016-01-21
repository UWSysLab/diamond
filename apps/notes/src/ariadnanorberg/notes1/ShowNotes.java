package ariadnanorberg.notes1;

import java.util.ArrayList;
import java.util.List;

import com.parse.FindCallback;
import com.parse.ParseException;
import com.parse.ParseObject;
import com.parse.ParseQuery;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toolbar;

public class ShowNotes extends ListActivity {
	private List<Note> posts;
	private Toolbar toolbar;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		Intent intent = getIntent();
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.shownotes);

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
                }
                return false;        
            }
		});
	}
	
	private void refreshNotesList() {
		// refreshes list of notes to current state
		ParseQuery<ParseObject> query = ParseQuery.getQuery("Post");
		
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
				}
			}
		});
		
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
	
}