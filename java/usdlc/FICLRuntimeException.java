package usdlc;

public class FICLRuntimeException extends RuntimeException {
    private Throwable source;

    public FICLRuntimeException(Throwable source) {
        super(source.getMessage());
        this.source = source;

    }

    public FICLRuntimeException(String message) {
        super(message);
    }

    public Throwable getSource() {
        return source;
    }
}
