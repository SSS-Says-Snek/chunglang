from utils import compile, run_compiled_program

class TestExpressions:
    def test_block_nested(self):
        compile("test/programs/block_nested.chung")
        out, _, _ = run_compiled_program()
        assert out == "2\n"
