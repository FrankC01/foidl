; ModuleID = "/Users/frankcastellucci/pfoidl/pfoidl/codegen.py"
target triple = "x86_64-apple-darwin17.7.0"
target datalayout = ""

define void @"main"() 
{
entry:
  %".2" = add i8 4, 4
  %".3" = sub i8 %".2", 2
  %".4" = bitcast [5 x i8]* @"fstr" to i8*
  %".5" = call i32 (i8*, ...) @"printf"(i8* %".4", i8 %".3")
  ret void
}

declare i32 @"printf"(i8* %".1", ...) 

@"fstr" = internal constant [5 x i8] c"%i \0a\00"