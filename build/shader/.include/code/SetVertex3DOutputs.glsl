

#ifndef USING_VERTEX_TEXTURE_UV
    color = _color;
#else
    uv = vec2(_uv.x , 1.0 - _uv.y);
#endif

normal = normalize(modelMatrix * vec4(_normal, 0.0)).rgb;
position = (modelMatrix * vec4(_positionInModel, 1.0)).rgb;
viewVector = _cameraPosition - position;

