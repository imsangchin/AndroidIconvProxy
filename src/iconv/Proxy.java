package iconv;

public class Proxy {
	public static native void init();
	
	static {
		System.loadLibrary("iconv");
	}
}
