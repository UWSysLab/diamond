import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.ReactiveManager;
import edu.washington.cs.diamond.ReactiveManager.TxnFunction;

public class Publisher {
    public static void main(String[] args) throws InterruptedException {
        if (args.length < 1) {
            System.err.println("usage: java Publisher config_file [message]");
            System.exit(1);
        }
        String configFile = args[0];
        String message = "Testing...";
        if (args.length >= 2) {
            message = args[1];
        }

        Diamond.DiamondInit(configFile, 1, 0);

        final Diamond.DString str = new Diamond.DString();
        Diamond.DObject.Map(str, "javareactivetest:str");

        final String finalMessage = message;
        ReactiveManager.execute_txn(new TxnFunction() {
            public void func(Object...args) {
                str.Set(finalMessage);
            }
        });
    }
}
