class LoxClass:
    def __init__(self, name: str) -> None:
        self.name = name

    def to_string(self):
        return self.name
