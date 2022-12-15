sandwich : str_0 ::= str_1 str_2
	sandwich.errors = case str_1, str_2 of
		| String, String -> None
		| _, _ -> Error("Expected two strings, instead got" ++ 
				str_1.type ++ "and" ++ str_2.type
		end;
	forwards to concat(str_1, concat(str_2, str_1));