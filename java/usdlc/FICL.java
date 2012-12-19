package usdlc;

import java.io.StringWriter;
import java.io.Writer;
import java.lang.Boolean;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Random;

/** @noinspection deprecation*/
public class FICL {
    private static final int RUNNING_WORD_STACK_DEPTH = 32;
    private static final int COMPILING_STACK_DEPTH = 16;
    private static final int OPERATING_STACK_DEPTH = 128;
    private static final int SECOND_STACK_DEPTH = 16;
    private static final int LOOP_STACK_DEPTH = 32;

    public StringBuilder errors = new StringBuilder(64);
    public boolean abort = false, isCompileMode = false;
    public boolean throwExceptions = true;

    public FICL() {
        init();
    }

    /**
     * Compile FORTH code. No execution, but a call to run afterwards will
     * run the code just compiled.
     */
    public boolean compile(String sourceToCompile) {
        isCompileMode = true;
        writer = new StringWriter();
        abort = false;
        source = sourceToCompile;
        sourceLength = source.length();
        sourcePointer = 0;
        compiling.clear();
        errors.setLength(0);
        String name;
        //noinspection NestedAssignment
        while (!abort && ((name = getWord()) != null)) {
            if (dictionary.containsKey(name)) {
                CompiledWord word = (CompiledWord) dictionary.get(name);
                // we call the word in compile mode
                if (word.immediate)
                    word.run();
                else
                    compileWord(word);
            } else if (!compileLiteral(name)) {
                abort("Unknown word: " + name);
                break;
            }
        }
        compilingWord = "[[compiled]]";
        compiled = new WordOfWords();
        isCompileMode = false;
        return !abort;
    }

    /**
     * Run the code just compiled.
     */
    public boolean run() {
        debuggingCompile = false;
        return run("[[compiled]]", compiled);
    }

    /**
     * compile and run source
     */
    public boolean run(String sourceCode) {
        return compile(sourceCode) && run();
    }

    boolean run(String name, Runnable actor) {
        runningWords.push(name);
        try {
            actor.run();
        } finally {
            runningWords.pop();
        }
        return !abort;
    }

    /**
     * Write to standard out
     */
    public void print(Object object) {
        String text = "";
        try {
            text = object.toString();
            writer.write(text);
        } catch (Exception e) {
            abort("print," + text, e);
        }
    }

    public void abort(String msg, Exception e) {
        if (throwExceptions) throw new RuntimeException(msg);
        abort(msg + ',' + e.toString());
    }

    public void abort(String msg) {
        if (isCompileMode)
            errors.append("compile,").append(compilingWord).append(',');
        else {
            errors.append("run,");
            for (int i = 1; i <= runningWords.depth; i++) {
                errors.append(runningWords.stack[i]).append("->");
            }
            errors.append(',');
        }
        errors.append(msg).append('\n');
        abort = true;
        reset();
    }

    public void reset() {
        stack.depth = secondStack.depth = 0;
        compilingStack.depth = runningWords.depth = 0;
    }

    public String toString() {
        return writer.toString();
    }

    public String getWord() {
        // start by dropping any leading space characters
        do {
            if (sourcePointer == sourceLength) return null;
        } while (Character.isSpace(source.charAt(sourcePointer++)));
        int first = sourcePointer - 1;
        // and go to where we have a space again
        do {
            if (sourcePointer == sourceLength) return source.substring(first);
        } while (!Character.isSpace(source.charAt(sourcePointer++)));

        String word = source.substring(first, sourcePointer - 1);
        if (debuggingCompile) print(word + ' ');
        return word;
    }

    private boolean compileLiteral(String name) {
	    int num = 0, sign = -1, idx = 0, len = name.length();
	    if (name.charAt(0) == '-') {
		    sign = idx = 1;
	    }
	    while (idx < len) {
	        num = num * 10 + ('0' - name.charAt(idx++));
	    }

	    compilePushWord(name, new Integer(sign * num));
	    return true;
    }

    public class Stack {
        Object[] stack;
        public int size = 0, depth = 0;

        public Stack(int depth) {
            stack = new Object[size = depth];
        }

        public Object peek() {
            return stack[depth];
        }

        public Object pop() {
            Object value = stack[depth];
            depth = (depth - 1) % size;
            return value;
        }

        public void push(Object value) {
            depth = (depth + 1) % size;
            stack[depth] = value;
        }

        public int popInt() {
            Object object = pop();
            try {
                int value = 0;
                if (object instanceof Boolean) {
                    if (((Boolean) object).booleanValue()) {
                        value = 1;
                    }
                } else {
                    value = ((Number) object).intValue();
                }
                return value;
            } catch (Exception e) {
                abort("pop Integer," + object, e);
            }
            return 0;
        }
    }

    private void compileWord(CompiledWord word) {
        compiling.add(word);
    }

    public void compileWord(String name, Runnable word) {
        compileWord(new CompiledWord(name, word));
    }

    public void compileWord(Runnable word) {
        compileWord(new CompiledWord(compilingWord, word));
    }

    public void compilePushWord(String name, Object value) {
        compileWord(compiledPushWord(name, value));
    }

    public class CompiledWord {
        public String name, source = "";
        public Runnable actor;
        public boolean immediate = false, variable = false;

        CompiledWord(String name, Runnable actor) {
            this.name = this.source = name;
            this.actor = actor;
        }

        public void run() {
            try {
                FICL.this.run(name, actor);
            } catch (Exception e) {
                abort("run", e);
            }
        }
    }

    private class ImmediateWord extends CompiledWord {
        ImmediateWord(String name, Runnable actor) {
            super(name, actor);
            immediate = true;
        }
    }

    private class VariableWord extends CompiledWord {
        CompiledWord value;

        VariableWord(String name, CompiledWord actor) {
            super(name, actor.actor);
            variable = true;
        }

        public void run() {
            value.run();
        }
    }

    private class WordOfWords implements Runnable {
        private CompiledWord[] words = (CompiledWord[]) compiling.toArray(new CompiledWord[compiling.size()]);

        WordOfWords() {
        }

        public void run() {
            int runPointer = 0;
            int end = words.length;
            while (runPointer < end) {
                if (abort) break;
                CompiledWord word = words[runPointer++];
                //System.out.println(word.name+"("+(runPointer-1)+")");
                word.run();
                if (jumpTo != 0) runPointer = jumpTo;
                jumpTo = 0;
            }
        }
    }

    private int jumpTo = 0;

    private String source = "";
    private int sourcePointer = 0, sourceLength = 0;
    public String compilingWord = "";
    private String lastDefinition = "";
    private Stack runningWords = new Stack(RUNNING_WORD_STACK_DEPTH);
    private ArrayList compiling = new ArrayList(16);
    public boolean debuggingCompile = false;
    private Runnable compiled = null;
    private Stack compilingStack = new Stack(COMPILING_STACK_DEPTH);
    public final Stack stack = new Stack(OPERATING_STACK_DEPTH);
    private final Stack secondStack = new Stack(SECOND_STACK_DEPTH);
    private final Stack loopStack = new Stack(LOOP_STACK_DEPTH);
    private Writer writer = new StringWriter();

    public class Dictionary extends Hashtable {
        public CompiledWord put(final String key, final CompiledWord value) {
            boolean wasVariable = (containsKey(key) && ((CompiledWord) get(key)).variable);
            if (wasVariable) {
                compileWord(new Runnable() {
                    public void run() {
                        ((VariableWord) get(key)).value = value;
                    }
                });
            } else {
                super.put(key, value);
            }
            return value;
        }
    }

    public final Dictionary dictionary = new Dictionary();

    /**
     * Extend FICL with a new word.
     */
    public void extend(String name, Runnable actor) {
        dictionary.put(name, new CompiledWord(name, actor));
    }

    public CompiledWord compiledPushWord(String name, final Object value) {
        return new CompiledWord(name, new Runnable() {
            public void run() {
                stack.push(value);
            }
        });
    }

    /**
     * Extend FICL with a new word.
     */
    public void immediate(String name, Runnable actor) {
        dictionary.put(name, new ImmediateWord(name, actor));
    }

    ////////////////////////////////////////////////////////
    private void init() {
        immediate(":", new Runnable() {
            public void run() {
                secondStack.push(compilingWord);
                secondStack.push(new Integer(sourcePointer));
                compilingWord = getWord();
                compilingStack.push(compiling);
                compiling = new ArrayList(16);
            }
        });
        immediate(";", new Runnable() {
            public void run() {
                int start = ((Integer) secondStack.pop()).intValue();
                final CompiledWord compiledWord =
                        new CompiledWord(compilingWord, new WordOfWords());
                compiledWord.source =
                        source.substring(start, sourcePointer);

                //noinspection unchecked
                compiling = (ArrayList) compilingStack.pop();
                lastDefinition = compilingWord;
                compilingWord = (String) secondStack.pop();
                dictionary.put(lastDefinition, compiledWord);
            }
        });
        immediate("(", new Runnable() {
            public void run() {
                sourcePointer = source.indexOf(')', sourcePointer) + 1;
                if (sourcePointer == 0) sourcePointer = sourceLength;
            }
        });
        immediate("\\", new Runnable() {
            public void run() {
                sourcePointer = sourceLength;
            }
        });
        immediate("\"", new Runnable() {
            public void run() {
                compilePushWord("\"", getQuotedString());
            }
        });
	    extend(".", new Runnable() {
            public void run() {
                print(stack.pop());
            }
        });
        extend("and", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() & stack.popInt()));
            }
        });
        extend("+", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() + stack.popInt()));
            }
        });
        immediate("begin", new Runnable() {
            public void run() {
                final int[] beginLeaveEnd = {compiling.size(), 0};
                loopStack.push(beginLeaveEnd);
            }
        });
        immediate("leave", new Runnable() {
            public void run() {
                final int[] beginLeaveEnd = (int[]) loopStack.peek();
                compileWord("leave", new Runnable() {
                    public void run() {
                        jumpTo = beginLeaveEnd[1];
                    }
                });
            }
        });
        immediate("?leave", new Runnable() {
            public void run() {
                final int[] beginLeaveEnd = (int[]) loopStack.peek();
                compileWord("?leave", new Runnable() {
                    public void run() {
                        int testResult = ((Number) stack.pop()).intValue();
                        if (testResult == 0) {
                            jumpTo = beginLeaveEnd[1];
                        }
                    }
                });
            }
        });
        immediate("again", new Runnable() {
            public void run() {
                final int[] beginLeaveEnd = (int[]) loopStack.pop();
                beginLeaveEnd[1] = compiling.size() + 1;
                compileWord("again", new Runnable() {
                    public void run() {
                        jumpTo = beginLeaveEnd[0];
                    }
                });
            }
        });
        immediate("constant", new Runnable() {
            public void run() {
                ((CompiledWord) dictionary.get(lastDefinition)).run();
                final Object constant = stack.pop();
                dictionary.put(lastDefinition, compiledPushWord(
                        lastDefinition + " - constant", constant));
            }
        });
        extend("dec", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() - 1));
            }
        });
        immediate("debug-compile", new Runnable() {
            public void run() {
                debuggingCompile = true;
            }
        });
        extend("/", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() / stack.popInt()));
            }
        });
        extend("drop", new Runnable() {
            public void run() {
                stack.pop();
            }
        });
        extend("dup", new Runnable() {
            public void run() {
                stack.push(stack.peek());
            }
        });
        extend("=", new Runnable() {
            public void run() {
                stack.push((stack.pop().equals(stack.pop())) ?
	                Boolean.TRUE : Boolean.FALSE);
            }
        });
        immediate("if", new Runnable() {
            public void run() {
                final int[] ifElseNext = {compiling.size(), 0, 0};
                secondStack.push(ifElseNext);
                compileWord("if", new Runnable() {
                    public void run() {
                        int testResult = stack.popInt();
                        if (testResult == 0) {
                            jumpTo = ifElseNext[1];
                        }
                    }
                });
            }
        });
        immediate("else", new Runnable() {
            public void run() {
                final int[] ifElseNext = (int[]) secondStack.peek();
                ifElseNext[1] = compiling.size() + 1; // over else
                compileWord("else", new Runnable() {
                    public void run() {
                        jumpTo = ifElseNext[2];
                    }
                });
            }
        });
        immediate("then", new Runnable() {
            public void run() {
                int[] ifElseNext = (int[]) secondStack.pop();
                // make sure we run the then statement to clean up secondStack
                ifElseNext[2] = compiling.size();
                if (ifElseNext[1] == 0) ifElseNext[1] = ifElseNext[2];
            }
        });
        immediate("immediate", new Runnable() {
            public void run() {
                ((CompiledWord) dictionary.get(lastDefinition)).immediate = true;
            }
        });
        extend("inc", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() + 1));
            }
        });
        extend("-", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() - stack.popInt()));
            }
        });
        extend("*", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() * stack.popInt()));
            }
        });
        extend("not", new Runnable() {
            public void run() {
                stack.push((stack.popInt() == 0) ? Boolean.TRUE : Boolean.FALSE);
            }
        });
        extend("or", new Runnable() {
            public void run() {
                stack.push(new Integer(stack.popInt() | stack.popInt()));
            }
        });
        immediate("push", new Runnable() {
            public void run() {
                compilePushWord("push", dictionary.get(getWord()));
            }
        });
        extend("random", new Runnable() {
            public void run() {
                int i = random.nextInt(stack.popInt());
                stack.push(new Integer(i));
            }
        });
        extend("return", new Runnable() {
            public void run() {
                jumpTo = 100000;
            }
        });
        immediate("variable", new Runnable() {
            public void run() {
                final CompiledWord word = (CompiledWord) dictionary.get(lastDefinition);
                if (!word.variable) {
                    dictionary.put(lastDefinition, new VariableWord(
                            lastDefinition + " - variable", word));
                    dictionary.put(lastDefinition, word);   // matches second
                }
            }
        });
    }

    private String getQuotedString() {
        int start = sourcePointer;
        sourcePointer = source.indexOf('"', sourcePointer);
        String text;
        if (sourcePointer == -1) {
            sourcePointer = sourceLength;
            text = source.substring(start);
        } else {
            text = source.substring(start, sourcePointer++);
        }
        return text;
    }

    private Random random = new Random();
}
