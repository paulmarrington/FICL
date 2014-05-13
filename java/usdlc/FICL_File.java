/* Copyright (C) 2013,14 paul@marrington.net, see /GPL license */
package usdlc;
import java.io.*;


public class FICL_File implements FICL_Persistence {
  private String basePath = ".";

  public void location(final String dir) { this.basePath = dir; }

  public String upload(String name, String contents) {
    try {
      Writer writer = new BufferedWriter(new OutputStreamWriter(
      new FileOutputStream(basePath+'/'+name), "utf-8"));
      writer.write(contents);
      writer.close();
    } catch (IOException ex) {
      return ex.toString();
    }
    return "";
  }
  public String load(String name) {
    try {
        RandomAccessFile raf = new RandomAccessFile(basePath+'/'+name, "r");
        byte[] buffer = new byte[(int)raf.length()];
        raf.readFully(buffer);
	      raf.close();
        return new String(buffer, "utf-8");
    } catch (IOException ex) {
      return "";
    }
  }
}
