# Sensoriando:: Sensor modules

É possível passar uma definição como #define MODULE SOIL na hora da compilação, sem alterar o código fonte diretamente, usando a flag de compilação -D.

##💡 Como fazer isso

Se você estiver usando Arduino CLI ou PlatformIO, veja como definir MODULE na linha de comando.
✅ Usando Arduino CLI

Se estiver usando a Arduino CLI, você pode passar a macro MODULE assim:

arduino-cli compile --build-properties build.extra_flags="-DMODULE=SOIL" --fqbn arduino:avr:uno .

    Troque arduino:avr:uno pela placa que você estiver usando.

##🔧 Modificar arquivos internos da IDE (não recomendado)

Você até poderia editar os arquivos de compilação da plataforma (platform.txt) e adicionar -DMODULE=SOIL

##🗂️ Onde está o platform.txt?

Se você instalou o core arduino:avr com a IDE ou Arduino CLI, o caminho será algo como:

~/.arduino15/packages/.../platform.txt

Procure pela linha que começa com: compiler.cpp.flags=
