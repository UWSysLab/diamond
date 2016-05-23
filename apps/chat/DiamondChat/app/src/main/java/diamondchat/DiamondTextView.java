package diamondchat;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.TextView;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.ReactiveManager;

public class DiamondTextView extends TextView {

	private class DiamondTextViewTransaction implements ReactiveManager.TxnFunction {
		DiamondTextView textView;
		DString dstr;
		public DiamondTextViewTransaction(DiamondTextView tv, DString ds) {
			textView = tv;
			dstr = ds;
		}
		@Override
		public void func(Object...objects) {
			final String value = dstr.Value();
			textView.post(new Runnable() {
				public void run() {
					textView.setText(value);
				}
			});
		}
	}
		
	public DiamondTextView(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
	}
	
	public void bindDString(String key) {
		DString dstr = new DString();
		Diamond.DObject.Map(dstr, key);
		ReactiveManager.reactive_txn(new DiamondTextViewTransaction(this, dstr));
	}
}