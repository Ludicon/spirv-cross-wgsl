var<private> FragColor : i32;

fn frag_main()
{
    switch (0)
    {
        default:
        {
            FragColor = 16;
            for (var _143 : i32 = 0; _143 < 25; )
            {
                FragColor += 10;
                _143++;
                continue;
            }
            for (var _144 : i32 = 1; _144 < 30; )
            {
                FragColor += 11;
                _144++;
                continue;
            }
            var _145 : i32;
            _145 = 0;
            for (; _145 < 20; )
            {
                FragColor += 12;
                _145++;
                continue;
            }
            var _62 : i32 = _145 + 3;
            FragColor += _62;
            if (_62 == 40)
            {
                for (var _149 : i32 = 0; _149 < 40; )
                {
                    FragColor += 13;
                    _149++;
                    continue;
                }
                break;
            }
            FragColor += _62;
            var _146 : vec2i;
            _146 = vec2i(0);
            for (; _146.x < 10; )
            {
                FragColor += _146.y;
                var _142 : vec2i = _146;
                _142.x = _146.x + 4;
                _146 = _142;
                continue;
            }
            for (var _148 : i32 = _62; _148 < 40; )
            {
                FragColor += _148;
                _148++;
                continue;
            }
            FragColor += _62;
            break;
        }
    }
}

struct SPIRV_Cross_Output
{
    @location(0) FragColor : i32,
}

@fragment
fn main() -> SPIRV_Cross_Output
{
    frag_main();
    var stage_output : SPIRV_Cross_Output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
