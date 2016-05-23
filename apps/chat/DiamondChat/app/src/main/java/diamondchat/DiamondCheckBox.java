package diamondchat;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CheckBox;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DBoolean;
import edu.washington.cs.diamond.ReactiveManager;

public class DiamondCheckBox extends CheckBox {
	private DBoolean dBool;
	
	private class DiamondCheckBoxTransaction implements ReactiveManager.TxnFunction {
		private DBoolean dBool;
		private DiamondCheckBox checkBox;
		
		public DiamondCheckBoxTransaction(DiamondCheckBox checkBox, DBoolean dBool) {
			this.dBool = dBool;
			this.checkBox = checkBox;
		}
		@Override
		public void func(Object... arg0) {
			final boolean value = dBool.Value();
			checkBox.post(new Runnable() {
				public void run() {
					checkBox.setChecked(value);
				}
			});
		}
	}
	
	public DiamondCheckBox(Context context, AttributeSet attrs) {
		super(context, attrs);
		dBool = new DBoolean();
	}
	
	public void bindDBoolean(String key) {
		Diamond.DObject.Map(dBool, key);
		ReactiveManager.reactive_txn(new DiamondCheckBoxTransaction(this, dBool));
		
		setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				ReactiveManager.execute_txn(new ReactiveManager.TxnFunction() {
					@Override
					public void func(Object... arg0) {
						dBool.Set(!dBool.Value());
					}
				});
			}
		});
	}
}
