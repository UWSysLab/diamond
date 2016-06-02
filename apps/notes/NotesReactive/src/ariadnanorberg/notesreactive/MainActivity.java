package ariadnanorberg.notesreactive;
// code based on tutorial from http://www.sitepoint.com/creating-cloud-backend-android-app-using-parse/

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
	}

	public void showNotes(View view) {
		// Goes to new view
		Intent viewNotes = new Intent(MainActivity.this, ShowNotes.class);
		MainActivity.this.startActivity(viewNotes);
	}
}