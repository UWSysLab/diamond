package ariadnanorberg.notesreactive;

import java.util.ArrayList;
import android.app.Activity;
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
import edu.washington.cs.diamond.Diamond;
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
	private Diamond.DStringList notesList;			
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_edit_note);
		
		Diamond.DiamondInit("128.208.6.85", "12444");
		ReactiveManager.StartManager();
		ReactiveManager.RegisterLogger(new ReactiveManager.Logger() {
			public void onLog(String message) {
				Log.i("NotesReactive", message);
			}
		});
		
		Intent intent = this.getIntent();
	    titleEditText = (TextView) findViewById(R.id.noteTitle);
	    contentEditText = (EditText) findViewById(R.id.noteContent);
	    
	    notesList = new Diamond.DStringList("notesreactive:noteslist");
	    
		contentEditText.setOnEditorActionListener(new EditText.OnEditorActionListener() {
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				System.out.println("texteditor works");
				EditText ev = (EditText)v;
				new Thread(new NoteSender(ev.getText().toString())).start();
				return true;
			}
		});
		
	    if (intent.getExtras() != null) {
	    	title = intent.getStringExtra("noteTitle");
	    	content = intent.getStringExtra("noteContent");
	        //titles = intent.getStringArrayListExtra("titles");
	        //contents = intent.getStringArrayListExtra("contents");
	        
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
	    
	    ReactiveManager.reactive_txn(new ReactiveManager.TxnFunction() {
			public void func(Object...objects) {
				//Perform Diamond accesses
				titles = new ArrayList<String>();
				contents = new ArrayList<String>();
				for (int i = 0; i < notesList.Size(); i++) {
					String[] noteParts = notesList.Value(i).split(System.getProperty("line.separator"));
					titles.add(noteParts[0]);
					contents.add(noteParts[1]);
				}
			}
		});
	    
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
				int index = notesList.Index(title + System.lineSeparator() + content);
				if (index != -1) { // note exists, isn't a new note
					notesList.Erase(index);
				}
				notesList.Append(title + System.lineSeparator() + noteContent);
				content = noteContent;
				committed = Diamond.DObject.TransactionCommit();
			}
		}
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
		Intent intent = new Intent(this, ShowNotes.class);
		int committed = 0;
		while (committed == 0) {
			Diamond.DObject.TransactionBegin();
			notesList.Remove(title + System.lineSeparator() + content);
			committed = Diamond.DObject.TransactionCommit();
		}
	    intent.putStringArrayListExtra("titles", titles);
	    intent.putStringArrayListExtra("contents", contents);
	    startActivity(intent);
	}
		
}
