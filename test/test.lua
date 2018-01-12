-- Lua configuration file used for testing purposes

x=700
y=300
z=x+y

b = true
s = "I am a string"
i = 44

color = { r=0.5, g=0.7, b=0 }
array = { 0.1, 0.2, 0.3 }

table = {
    float = 0.2,
    int = 5,
    string = "hello there",
    bool = true,
    table = {
        string = "nested",
    },
    other_table = {
        string = "other nested",
    },
}

function f(a,b)
    return (a * math.sin(b))
end



