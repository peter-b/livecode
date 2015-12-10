script "revCommonMark"
constant kCodeIndent = 4

function MarkdownToHTML pText
   local tState
   ParseStart tState
   ParseText tState, pText
   ParseFinish tState
   
   local tHtml
   put empty into tHtml
   HtmlGenerate tState, tHtml
   return tHtml
end MarkdownToHTML

function CommonMarkToHTML pText
   return MarkdownToHTML(pText)
end CommonMarkToHTML

----------------------------------------------------------------
-- Top-level parser API
----------------------------------------------------------------

private command ParseStart @xState
   put EmptyArray() into xState
   
   put empty into xState["line"]
   
   -- Create document block
   BlockSet xState, "doc", "type", "document"
   BlockSet xState, "doc", "open", true
   
   -- Current "tip" block is the document block
   TipSet xState, "doc"
end ParseStart

private command ParseText @xState pText   
   repeat for each line tLine in pText
      ParseLine xState, tLine
   end repeat
end ParseText

private command ParseLine @xState pLine
   LogDebug merge("ParseLine '[[pLine]]'")
   
   -- Replace NUL characters for security
   replace numtochar(0) with numtochar(0xfffd) in pLine
   
   put pLine into xState["line"]
   put 0 into xState["offset"]
   put 0 into xState["column"]
   
   put TipGet(xState) into xState["oldTipPath"]
   
   ---------- Check whether any blocks can be closed   
   -- Iterate through open blocks, checking that their expected
   -- line start is present.  <tContainerPath> will contain the
   -- last open matching block.  If not all open blocks matched,
   -- then <tAllMatched> will be false.
   local tContainerPath, tChildPath, tAllMatched
   put "doc" into tContainerPath
   put true into tAllMatched
   
   repeat forever
      put BlockLastChild(xState, tContainerPath) into tChildPath
      
      if tChildPath is empty or not BlockIsOpen(xState, tChildPath) then
         exit repeat
      end if
      
      put tChildPath into tContainerPath
      
      LineFindNextNonSpace xState
      
      switch BlockContinues(xState, tContainerPath)
         case "matched"
            break
         case "unmatched"
            put false into tAllMatched
            break
         default
            throw "Bad result from BlockContinues()"
      end switch
      
      if not tAllMatched then
         put BlockParent(xState, tContainerPath) into tContainerPath
         exit repeat
      end if
      
   end repeat
   
   ---------- Store stuff for ParseCloseUnmatchedBlocks
   -- xState["lastMatchedContainer"] is the path of the last matched
   -- block this time round
   put tContainerPath into xState["lastMatchedContainer"]
   -- xState["allClosed"] is true when all blocks that need to be
   -- closed for this line have already been closed
   put tContainerPath is xState["oldTipPath"] into xState["allClosed"]
   
   ---------- Work out if any new blocks need to be opened
   -- Repeatedly try new container starts, adding any children to the
   -- last matched container
   local tFoundLeaf
   put (BlockType(xState, tContainerPath) is not "paragraph" and \
         BlockAcceptsLines(xState, tContainerPath)) into tFoundLeaf
   
   repeat while not tFoundLeaf
      
      LineFindNextNonSpace xState
      
      switch BlockStarts(xState, tContainerPath)
         case "unmatched"
            LineAdvanceNextNonSpace xState
            exit repeat
         case "container"
            put TipGet(xState) into tContainerPath
            break
         case "leaf"
            put TipGet(xState) into tContainerPath
            put true into tFoundLeaf
            break
         default
            throw "Bad result from BlockStarts()"
      end switch
      
   end repeat
   
   ---------- Figure out where to put the actual contents of the line
   -- Anything that remains in the line is a text line.  Add it to the
   -- correct container
   
   if not xState["allClosed"] and not LineIsBlank(xState) and \
         TipType(xState) is "paragraph" then
      -- This is a lazy paragraph continuation
      TipAddLine xState
      
   else
      -- Not a lazy paragraph continuation, so add it to a container as
      -- normal
      ParseCloseUnmatchedBlocks xState
      
      if TipAcceptsLines(xState) then
         TipAddLine xState
         
      else if (not LineIsBlank(xState)) and LineHasRemainder(xState) then
         -- Create a new paragraph block for the line
         TipAddChild xState, "paragraph"
         LineAdvanceNextNonSpace xState
         TipAddLine xState
      end if
   end if
end ParseLine

private command ParseFinish @xState
   LogDebug("ParseFinish")
   repeat while TipGet(xState) is not empty
      BlockFinish xState, TipGet(xState)
   end repeat
   return ""
end ParseFinish

private command ParseCloseUnmatchedBlocks @xState
   if xState["allClosed"] then
      exit ParseCloseUnmatchedBlocks
   end if
      
   local tParent
   repeat while xState["oldTipPath"] is not xState["lastMatchedContainer"]
      put BlockParent(xState, xState["oldTipPath"]) into tParent
      BlockFinish xState, xState["oldTipPath"]
      put tParent into xState["oldTipPath"]
   end repeat
end ParseCloseUnmatchedBlocks

----------------------------------------------------------------
-- Parser block handling
----------------------------------------------------------------

private function BlockGet pState, pPath, pParam
   if pPath is empty then
      throw merge("BlockGet: invalid path '[[pPath]]'")
   end if
   return pState["model"][pPath][pParam]
end BlockGet

private command BlockSet @xState, pPath, pParam, pValue
   if pPath is empty then
      throw merge("BlockSet: invalid path '[[pPath]]'")
   end if
   put pValue into xState["model"][pPath][pParam]
end BlockSet

private function BlockType pState, pPath
   return BlockGet(pState, pPath, "type")
end BlockType

private function BlockIsOpen pState, pPath
   return BlockGet(pState, pPath, "open") is true
end BlockIsOpen

private function BlockChildren pState, pPath
   return BlockGet(pState, pPath, "children")
end BlockChildren

private function BlockLastChild pState, pPath
   return line -1 of BlockGet(pState, pPath, "children")
end BlockLastChild

private function BlockFirstChild pState, pPath
   return line 1 of BlockGet(pState, pPath, "children")
end BlockFirstChild

private function BlockParent pState, pPath
   return item 1 to -2 of pPath
end BlockParent

private command BlockAddLine @xState, pPath, pLineData
   local tStringContents
   put BlockGet(xState, pPath, "stringContent") into tStringContents
   put pLineData & return after tStringContents
   BlockSet xState, pPath, "stringContent", tStringContents
end BlockAddLine

private function BlockLiteralContent pState, pPath
   return BlockGet(pState, pPath, "literal")
end BlockLiteralContent

private function BlockStringContent pState, pPath
   return BlockGet(pState, pPath, "stringContent")
end BlockStringContent

private function BlockContinues @xState, pPath
   local tType
   
   put BlockType(xState, pPath) into tType
   
   switch tType
      case "document"
         return "matched"
      case "paragraph"
         return BlockContinues__paragraph(xState, pPath)  
      case "codeblock"
         return BlockContinues__codeblock(xState, pPath)
      default
         throw "BlockContinues: bad block type" && BlockType(xState, pPath)
   end switch
end BlockContinues

private function BlockContinues__paragraph @xState, pPath
   if LineIsBlank(xState) then
      return "unmatched"
   else
      return "matched"
   end if
end BlockContinues__paragraph

private function BlockContinues__codeblock @xState, pPath
   if LineIndentCode(xState) then
      LineAdvanceColumns xState, kCodeIndent
   else if LineIsBlank(xState) then
      LineAdvanceNextNonSpace xState
   else
      return "unmatched"
   end if
   return "matched"
end BlockContinues__codeblock

private command BlockFinish @xState, pPath
   LogDebug "BlockFinish" && pPath && BlockType(xState, pPath)
   
   BlockSet xState, pPath, "open", false
   
   local tStringContent
   switch BlockType(xState, pPath)
      case "document"
         break
         
      case "paragraph"
         -- FIXME support for link reference definitions
         BlockFinish__paragraph xState, pPath
         break
         
      case "codeblock"
         BlockFinish__codeblock xState, pPath
         break
         
      default
         throw "BlockFinish: bad block type:" && BlockType(xState, pPath)
   end switch
   
   TipSet xState, BlockParent(xState, pPath)
end BlockFinish

private command BlockFinish__paragraph @xState, pPath
   local tContent
   put BlockStringContent(xState, pPath) into tContent
   
   -- Remove trailing blanks
   BlockSet xState, pPath, "stringContent", word 1 to -1 of tContent
end BlockFinish__paragraph

private command BlockFinish__codeblock @xState, pPath
   local tContent
   put BlockStringContent(xState, pPath) into tContent
   
   -- Remove trailing blank lines from the code block
   repeat while the number of lines in tContent > 1
      if word 1 to -1 of (line -1 of tContent) is not empty then
         exit repeat
      else
         delete line -1 of tContent
      end if
   end repeat
   
   BlockSet xState, pPath, "literal", tContent
   BlockSet xState, pPath, "stringContent", empty
end BlockFinish__codeblock

private function BlockAcceptsLines @xState, pPath
   switch BlockType(xState, pPath)
      case "document"
         return false
         
      case "paragraph"
         return true
         
      case "codeblock"
         return true
         
      default
         throw "BlockAcceptsLines: bad block type:" && BlockType(xState, pPath)
   end switch
end BlockAcceptsLines

private function BlockCanContain pState, pPath, pTag
   switch BlockType(pState, pPath)
      case "document"
         return pTag is not "item"
      case "paragraph"
      case "codeblock"
         return false
      default
         throw "BlockCanContain: bad block type:" && BlockType(pState, pPath)
   end switch
end BlockCanContain

private command BlockAddChild @xState, pPath, pTag
   if not BlockCanContain(xState, pPath, pTag) then
      throw "Bad child:" && BlockType(xState, pPath) && pTag
   end if
   
   local tChildren
   put BlockGet(xState, pPath, "children") into tChildren
   
   local tChildPath
   put pPath & comma into tChildPath
   put 1 + the number of lines in tChildren after tChildPath
   
   BlockSet xState, tChildPath, "type", pTag
   BlockSet xState, tChildPath, "open", true
   
   put tChildPath & return after tChildren
   BlockSet xState, pPath, "children", tChildren
   
   LogDebug merge("BlockStart [[tChildPath]] [[pTag]]")
end BlockAddChild

-- Returns "unmatched", "container" or "leaf"
private function BlockStarts @xState, pParent
   local tStatus
   if BlockStarts__codeblock(xState, pParent, tStatus) then return tStatus
   return "unmatched"
end BlockStarts

private function BlockStarts__codeblock @xState, pParent, @rStatus
   if not LineIndentCode(xState) or \
         TipType(xState) is "paragraph"  or \
         LineIsBlank(xState) then
      return false
   end if
   
   LineAdvanceColumns xState, kCodeIndent
   ParseCloseUnmatchedBlocks xState
   TipAddChild xState, "codeblock"
   put "leaf" into rStatus
   return true
end BlockStarts__codeblock

private function TipType pState
   return BlockType(pState, TipGet(pState))
end TipType

private command TipAddLine @xState
   BlockAddLine xState, TipGet(xState), LineRemainder(xState)
end TipAddLine

private function TipAcceptsLines pState
   return BlockAcceptsLines(pState, TipGet(pState))
end TipAcceptsLines

private command TipAddChild @xState, pTag
   local tPath
   put TipGet(xState) into tPath
   
   -- Close blocks until we find a block that accepts <pTag>
   repeat while not BlockCanContain(xState, tPath, pTag)
      BlockFinish xState, tPath
      put BlockParent(xState, tPath) into tPath
      if tPath is empty then
         throw "Inconsistent state in TipAddChild!"
      end if
   end repeat
      
   BlockAddChild xState, tPath, pTag
   
   TipSet xState, BlockLastChild(xState, tPath)
end TipAddChild

private function TipGet pState
   return pState["tipPath"]
end TipGet

private command TipSet @xState, pPath
   put pPath into xState["tipPath"]
end TipSet

----------------------------------------------------------------
-- Parser lexing
----------------------------------------------------------------

private function LineIsBlank pState
   return pState["blank"]
end LineIsBlank

private function LineHasRemainder pState
   return LineRemainder(pState) is not empty
end LineHasRemainder

private function LineRemainder pState
   return char (pState["offset"] + 1) to -1 of pState["line"]
end LineRemainder

private command LineFindNextNonSpace @xState
   local tOffset, tChar, tLine, tColumn
   put xState["offset"] into tOffset
   put xState["line"] into tLine
   put xState["column"] into tColumn
   
   repeat forever
      put char (tOffset+1) of tLine into tChar
      switch tChar
         case empty
            exit repeat
         case space
            add 1 to tOffset
            add 1 to tColumn
            break
         case tab
            add 1 to tOffset
            add (4 - tColumn mod 4) to tColumn
            break
         default
            exit repeat
      end switch
   end repeat
   
   put (tChar is empty) into xState["blank"]
   put tOffset into xState["nextNonSpace"]
   put tColumn into xState["nextNonSpaceColumn"]
   put xState["nextNonSpaceColumn"] - xState["column"] into xState["indent"]
end LineFindNextNonSpace

private command LineAdvanceNextNonSpace @xState
   put xState["nextNonSpace"] into xState["offset"]
   put xState["nextNonSpaceColumn"] into xState["column"]
end LineAdvanceNextNonSpace

private function LineIndent pState
   return pState["indent"]
end LineIndent

private function LineIndentCode pState
   return LineIndent(pState) >= kCodeIndent
end LineIndentCode

private command LineAdvanceOffset @xState, pCount, pIsColumns
   local tOffset = 0
   local tColumn = 0
   
   if pIsColumns is empty then
      put false into pIsColumns
   end if
   
   repeat while (pIsColumns and tColumn < pCount) or \
         (not pIsColumns and tOffset < pCount)
      
      if char (xState["offset"] + tOffset + 1) of xState["line"] is tab then
         add (4 - (tColumn + xState["column"]) mod 4) to tColumn
      else
         add 1 to tColumn
      end if
      add 1 to tOffset
   end repeat
   
   add tOffset to xState["offset"]
   add tColumn to xState["column"]
end LineAdvanceOffset

private command LineAdvanceColumns @xState, pCount
   LineAdvanceOffset xState, pCount, true
end LineAdvanceColumns

----------------------------------------------------------------
-- HTML generation
----------------------------------------------------------------

private command HtmlGenerate pState, @xHtml, pPath
   if pPath is empty then
      put "doc" into pPath
   end if
   
   local tTag
   put BlockType(pState, pPath) into tTag
   
   switch tTag
      case "document"
         break
      case "paragraph"
         put "<p>" after xHtml
         break
      case "codeblock"
         put "<pre><code>" after xHtml
         break
      default
         throw "HtmlGenerate: bad block type:" && tTag
   end switch
   
   if tTag is among the items of "document" then
      local tChildPath
      repeat for each line tChildPath in BlockChildren(pState, pPath)
         HtmlGenerate pState, xHtml, tChildPath
      end repeat
   else if tTag is among the items of "paragraph" then
      -- FIXME inline tags
      put BlockStringContent(pState, pPath) after xHtml
   else if tTag is among the items of "codeblock" then
      HtmlLiteralEscape xHtml, BlockLiteralContent(pState, pPath)
   end if
   
   switch tTag
      case "document"
         break
      case "paragraph"
         put "</p>" & return after xHtml
         break
      case "codeblock"
         put "</code></pre>" & return after xHtml
         break
   end switch
end HtmlGenerate

private command HtmlLiteralEscape @xHtml, pLiteral
   repeat for each char tChar in pLiteral
      switch tChar
         case quote
            put "&quot;" after xHtml
            break
         case "<"
            put "&lt;" after xHtml
            break
         case ">"
            put "&gt;" after xHtml
            break
         case "&"
            put "&amp;" after xHtml
            break
         default
            put tChar after xHtml
      end switch
   end repeat
end HtmlLiteralEscape

----------------------------------------------------------------
-- Misc utilities
----------------------------------------------------------------

constant kDebug = true

private command LogDebug pMessage
   if not kDebug then
      exit LogDebug
   end if
   
   repeat for each line tLine in pMessage
      write "debug:" && tLine & return to stdout
   end repeat
end LogDebug

private command ArrayPush @xArray, pValue
   local tCount
   put the number of elements in xArray into tCount
   put pValue into xArray[tCount+1]
end ArrayPush

private command ArrayPop @xArray
   local tCount
   put the number of elements in xArray into tCount
   if tCount > 0 then
      delete variable xArray[tCount]
   end if
end ArrayPop

private function ArrayIsEmpty pArray
   return 0 is the number of elements in pArray
end ArrayIsEmpty

private function EmptyArray
   local tArray
   put 1 into tArray[1]
   delete variable tArray[1]
   return tArray
end EmptyArray
