script "TestCommonMarkLibrary"
/*
Copyright (C) 2015 LiveCode Ltd.

This file is part of LiveCode.

LiveCode is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License v3 as published by the Free
Software Foundation.

LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

local sCommonMarkTestData

constant kTestJsonFile = "CommonMark-tests-0.22.json"

on TestSetup
   local tCommonMarkLib
   TestLoadExtension "com.livecode.library.json"

   put TestGetEngineRepositoryPath() & "/ide-support/revcommonmark.livecodescript" into tCommonMarkLib
   start using stack tCommonMarkLib

   -- This uses the commonmark.org testsuite.  You can generate a set
   -- of tests as JSON by cloning the
   -- https://github.com/jgm/CommonMark git repository and running:
   --
   --     python3 tests/spec_tests.py --dump-tests

   local tDirectory, tTestDataFile, tTestJson
   set the itemDelimiter to slash
   put item 1 to -2 of the filename of this stack into tDirectory

   put tDirectory & slash & kTestJsonFile into tTestDataFile

   open file tTestDataFile for "UTF-8" text read
   read from file tTestDataFile until end
   if the result is not empty and the result is not "eof" then
      throw merge("Failed to read [[kTestJsonFile]]: [[the result]]")
   end if
   put it into tTestJson
   close file tTestDataFile

   put JsonImport(tTestJson) into sCommonMarkTestData
end TestSetup

on TestCommonMark
   TestPlan the number of elements in sCommonMarkTestData

   local tTestInfo, tDescription, tOk
   repeat for each element tTestInfo in sCommonMarkTestData
      put "Example" && tTestInfo["example"] into tDescription
      put " (" & tTestInfo["section"] & ")" after tDescription

      local tHtml
      put MarkdownToHTML(tTestInfo["markdown"]) into tHtml
      TestDiagnostic tHtml

      put (tHtml is tTestInfo["html"]) into tOk

      TestAssert tDescription, tOk
   end repeat
end TestCommonMark
