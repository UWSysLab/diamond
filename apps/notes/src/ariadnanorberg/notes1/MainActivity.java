package ariadnanorberg.notes1;

import com.parse.Parse;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		Parse.enableLocalDatastore(this);
		Parse.initialize(new Parse.Configuration.Builder(this)
			.applicationId("WsCxdr90gpqXO5rPQ6FvVJE46X8Tghr48sEUMaDN")
			.clientKey("qessDyZkCChhXdKo6BZRhiFk8ZHaKPjJEQDMhVin")
			.server("http://localhost:1337/parse")
			.build()
		);
		final Button viewNotes = (Button) findViewById(R.id.viewNotes);
	}

	public void showNotes(View view) {
		// Goes to new view
		Intent viewNotes = new Intent(MainActivity.this, ShowNotes.class);
		MainActivity.this.startActivity(viewNotes);
	}

}
