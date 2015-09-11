import edu.washington.cs.diamond.Diamond.DStringList;
import edu.washington.cs.diamond.Diamond.DString;

public class Main {
	public static void main(String[] args) {
		basicTest();
	}
	
	/**
	 * Simple test of Diamond syncing
	 */
	public static void basicTest() {
		DString testString1 = new DString("Diamond syncing not working", "javatest:str");
		DString testString2 = new DString("Diamond syncing not working", "javatest:str");
		
		testString1.Lock();
		testString1.Set("Diamond syncing is working");
		testString1.Broadcast();
		testString1.Unlock();
		
		System.out.println(testString2.Value());
	}
	
	/**
	 * Multithreaded ping-pong test
	 */
	public static void threadTest() {
		final DString pingPong = new DString("ping", "javatest:pingpong");
		new Thread(new Runnable() {
			public void run() {
				while (true) {
					pingPong.Lock();
					while (pingPong.Value().equals("ping")) {
						pingPong.Wait();
					}
					pingPong.Set("ping");
					pingPong.Signal();
					pingPong.Unlock();
					System.out.println("ping");
				}
			}
		}).start();
		new Thread(new Runnable() {
			public void run() {
				while (true) {
					pingPong.Lock();
					while (pingPong.Value().equals("pong")) {
						pingPong.Wait();
					}
					pingPong.Set("pong");
					pingPong.Signal();
					pingPong.Unlock();
					System.out.println("pong");
				}
			}
		}).start();
	}
	
	/**
	 * Tests bug that appeared in DiMessage when the chat log grew beyond a certain size
	 */
	public static void listSizeTest() {
		DStringList testList = new DStringList("javatest:testlist");
		testList.Clear();
		testList.Append("Nexus 7: hello from the Nexus 7");
		testList.Append("Nexus 5: Nexus 5 here");
		testList.Append("Nexus 7: Now to get sync working...");
		testList.Append("Nexus 5: what happened");
		testList.Append("Nexus 7: hello");
		testList.Append("Nexus 5: test");
		testList.Append("Nexus 5: testing locks");
		testList.Append("Nexus 5: testing more");
		testList.Append("Nexus 5: broadcast was causing segfaults");
		testList.Append("Nexus 5: trying broadcast again");
		System.out.println(testList.Value(0));
		System.out.println(testList.Size());
	}
}
