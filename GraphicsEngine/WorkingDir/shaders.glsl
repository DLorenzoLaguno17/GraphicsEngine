///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = texture(uTexture, vTexCoord);
}

#endif
#endif

// -----------------------------------------------------------------
// MESH SHADER
// -----------------------------------------------------------------

#ifdef SHOW_TEXTURED_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

struct Light
{
	vec3 color;
	vec3 direction;
	vec3 position;
	unsigned int type;
};

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition; // In worldspace
out vec3 vNormal;	// In worldspace
out vec3 vViewDir;	// In worldspace

void main()
{
	vTexCoord = aTexCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal = vec3(uWorldMatrix * vec4(aNormal, 0.0));
	vViewDir = uCameraPosition - vPosition;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Light
{
	vec3 color;
	vec3 direction;
	vec3 position;
	unsigned int type;
};

in vec2 vTexCoord;
in vec3 vPosition; // in worldspace
in vec3 vNormal; // in worldspace
in vec3 uViewDir; // in worldspace

uniform sampler2D uTexture;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(location = 0) out vec4 oColor;

void main()
{	
	// Mat parameters
    vec3 specular = vec3(1.0);	// color reflected by mat
    float shininess = 40.0;		// how strong specular reflections are (more shininess harder and smaller spec)
	vec4 albedo = texture(uTexture, vTexCoord);

	// Ambient
    float ambientIntensity = 0.25;
    vec3 ambientColor = albedo.xyz * ambientIntensity;

    vec3 N = normalize(vNormal);		// normal
	vec3 V = normalize(-uViewDir.xyz);	// direction from pixel to camera

	vec3 diffuseColor;
	vec3 specularColor;

	for(int i = 0; i < uLightCount; ++i)
	{
	    float attenuation = 1.0f;
		
		// If we have a point light, attenuate according to distance
		if(uLight[i].type == 1)
			attenuation = 1.0 / length(uLight[i].position - vPosition);
	        
	    vec3 L = normalize(uLight[i].direction - uViewDir.xyz); // Light direction 
	    vec3 R = reflect(-L, N);								// reflected vector
	    
	    // Diffuse
	    float diffuseIntensity = max(0.0, dot(N, L));
	    diffuseColor += attenuation * albedo.xyz * uLight[i].color * diffuseIntensity;
	    
	    // Specular
	    float specularIntensity = pow(max(dot(R, V), 0.0), shininess);
	    specularColor += attenuation * specular * uLight[i].color * specularIntensity;
	}

	oColor = vec4(ambientColor + diffuseColor + specularColor, 1.0);
}

#endif
#endif

// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.