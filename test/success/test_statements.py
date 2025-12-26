from utils import compile, run_compiled_program

class TestStatements:
    def test_explicit_return(self):
        compile("test/programs/explicit_return.chung")
        out, _, _ = run_compiled_program()
        assert int(out) == 3

    def test_while_0_to_10(self):
        compile("test/programs/while_0_to_10.chung")
        out, _, _ = run_compiled_program()
        assert out == "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n"

    def test_variable_initialization(self):
        compile("test/programs/variable_initialization.chung")
        out, _, _ = run_compiled_program()
        assert out == "1\n2\n"
