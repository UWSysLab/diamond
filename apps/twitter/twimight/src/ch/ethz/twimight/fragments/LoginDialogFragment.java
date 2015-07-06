package ch.ethz.twimight.fragments;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import ch.ethz.twimight.R;
import ch.ethz.twimight.activities.LoginActivity;

//TODO: written by Niel
public class LoginDialogFragment extends DialogFragment {

	public interface LoginDialogListener {
		public void onDialogPositiveClick(DialogFragment dialog);
	}
	
	LoginDialogListener mListener;
	View mDialogView;
	
	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		try {
			mListener = (LoginDialogListener)activity;
		} catch (ClassCastException e) {
			throw new ClassCastException(activity.toString() + " must implement LoginDialogListener");
		}
	}
	
	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
		LayoutInflater inflater = getActivity().getLayoutInflater();
		
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		mDialogView = inflater.inflate(R.layout.dialog_login, null);
		builder.setView(mDialogView)
			   .setTitle(R.string.dialog_login_title)
			   .setPositiveButton(R.string.dialog_login_positive, new DialogInterface.OnClickListener() {
				   public void onClick(DialogInterface dialog, int id) {
					   EditText screennameText = (EditText) mDialogView.findViewById(R.id.login_screenname);
					   EditText usernameText = (EditText) mDialogView.findViewById(R.id.login_username);
					   LoginActivity.setTwitterScreenname(screennameText.getText().toString(),
							   							  getActivity().getBaseContext());
					   LoginActivity.setTwitterUsername(usernameText.getText().toString(),
	   							getActivity().getBaseContext());
					   mListener.onDialogPositiveClick(LoginDialogFragment.this);
				   }
			   });
		return builder.create();
	}
}
