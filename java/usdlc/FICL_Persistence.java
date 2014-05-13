/* Copyright (C) 2013 paul@marrington.net, see /GPL license */
package usdlc;

public interface FICL_Persistence {
  public void location(String dir);
  public String upload(String name, String contents);
  public String load(String name);
}
