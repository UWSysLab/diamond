package ch.ethz.twimight.fragments;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;

import ch.ethz.twimight.R;

//TODO: written by Niel
public class LoginDialogFragment extends DialogFragment {

	public interface LoginDialogListener {
		public void onDialogPositiveClick(DialogFragment dialog);
	}
	
	LoginDialogListener mListener;
	
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
		builder.setView(inflater.inflate(R.layout.dialog_login, null))
			   .setTitle("Login")
			   .setPositiveButton("Confirm", new DialogInterface.OnClickListener() {
				   public void onClick(DialogInterface dialog, int id) {
					   mListener.onDialogPositiveClick(LoginDialogFragment.this);
				   }
		});
		return builder.create();
	}
}
