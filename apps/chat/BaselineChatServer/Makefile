all:
	javac -classpath libs/commons-pool2-2.0.jar:libs/gson-2.3.1.jar:libs/jedis-2.4.2.jar src/Main.java
	jar cfm baselinechatserver.jar src/MANIFEST.MF -C src/ .

clean:
	rm src/*.class
	rm baselinechatserver.jar
