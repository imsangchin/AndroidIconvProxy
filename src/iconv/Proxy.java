package iconv;

public class Proxy {
	private static native void init();
	
	static {
		System.loadLibrary("iconv");
		init();
	}
}
