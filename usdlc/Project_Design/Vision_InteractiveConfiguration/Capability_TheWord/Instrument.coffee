include 'Instrument'
ficl = session.instance usdlc.FICL

gwt /an initialised instance of FICL/, (all) ->
    ficl.reset()
gwt /(we run the FICL statements?|and) "(.+)"/, (all, what, code) ->
    throw ficl.errors.toString() if not ficl.run code
gwt /the FICL stack has a depth of (\d+)/, (all, depth) ->
    if ficl.stack.depth != +depth
        throw "Stack depth is #{ficl.stack.depth} (#{ficl.errors.toString()})"
gwt /FICL output is "(.+)"$/, (all, text) ->
    output = String(ficl.toString())
    throw "Output was '#{output}', not '#{text}' as expected" if output != text