package ariadnanorberg.notesreactive;

import java.util.ArrayList;
import java.util.Date;
import java.util.Map;
import java.util.TreeMap;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DObject;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.ReactiveManager;

public class EditNoteActivity extends Activity {
	private String title;
	private String content;
	private ArrayList<String> titles;
	private ArrayList<String> contents;
	private TextView titleEditText;
	private EditText contentEditText;
	private Button saveNoteButton;
	private Button deleteButton;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_edit_note);
		
		Intent intent = this.getIntent();
	    titleEditText = (TextView) findViewById(R.id.noteTitle);
	    contentEditText = (EditText) findViewById(R.id.noteContent);
	    
		contentEditText.setOnEditorActionListener(new EditText.OnEditorActionListener() {
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				EditText ev = (EditText)v;
				new Thread(new NoteSender(ev.getText().toString())).start();
				ev.setText("");
				return true;
			}
		});
		
	    if (intent.getExtras() != null) {
	    	title = intent.getStringExtra("noteTitle");
	    	content = intent.getStringExtra("noteContent");
	        titles = intent.getStringArrayListExtra("titles");
	        contents = intent.getStringArrayListExtra("contents");
	        
	        titleEditText.setText(title);
	        contentEditText.setText(content);
	    }

	    saveNoteButton = (Button)findViewById(R.id.saveNote);
	    saveNoteButton.setOnClickListener(new View.OnClickListener() {
	        @Override
	        public void onClick(View v) {
	            saveExit();
	        }
	    });
	    
	    deleteButton = (Button)findViewById(R.id.delete);
	    deleteButton.setOnClickListener(new View.OnClickListener() {
	    	@Override
	    	public void onClick(View v) {
	    		deleteNote();
	    	}
	    });
	    
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.edit_note, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	
	private void saveExit() {
		Intent intent = new Intent(this, ShowNotes.class);
	    intent.putStringArrayListExtra("titles", titles);
	    intent.putStringArrayListExtra("contents", contents);
	    startActivity(intent);
	}
	
	private void deleteNote() {
		titles.remove(title);
		contents.remove(content);
		saveExit();
	}
	
	private class NoteSender implements Runnable {
		String noteContent;
		public NoteSender(String content) {
			noteContent = content;
		}
		public void run() {
			int committed = 0;
			while(committed == 0) {
				Diamond.DObject.TransactionBegin();
				int index = titles.indexOf(title);
				if (index != -1) { // note exists, isn't a new note
					notesList.Remove(note);
				}
				notesList.Append(noteContent);
				committed = Diamond.DObject.TransactionCommit();
			}
		}
	}
		
}
