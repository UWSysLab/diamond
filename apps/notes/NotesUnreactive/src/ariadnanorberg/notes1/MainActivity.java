package ariadnanorberg.notes1;
// code based on tutorial from http://www.sitepoint.com/creating-cloud-backend-android-app-using-parse/

import com.parse.Parse;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		Parse.enableLocalDatastore(this);
		Parse.initialize(new Parse.Configuration.Builder(this)
			.applicationId("WsCxdr90gpqXO5rPQ6FvVJE46X8Tghr48sEUMaDN")
			.clientKey("qessDyZkCChhXdKo6BZRhiFk8ZHaKPjJEQDMhVin")
			.server("http://128.208.6.85:1337/parse/")
			.build()
		);
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
	}

	public void showNotes(View view) {
		// Goes to new view
		Intent viewNotes = new Intent(MainActivity.this, ShowNotes.class);
		MainActivity.this.startActivity(viewNotes);
	}
}