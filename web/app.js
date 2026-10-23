// Liga a página ao WebAssembly. O Module é criado no index.html
(function () {
    'use strict';

    const fileInput = document.getElementById('file-input');
    const dropzone = document.querySelector('.dropzone');
    const frame = document.querySelector('.image-frame');
    const status = document.getElementById('status');

    let runtimeReady = false;
    // Conta entradas e saídas porque cada elemento filho dispara dragleave
    let dragDepth = 0;

    Module.onRuntimeInitialized = function () {
        runtimeReady = true;
    };

    function showStatus(message, isError) {
        status.textContent = message;
        status.classList.toggle('is-error', isError);
    }

    async function loadFile(file) {
        if (!runtimeReady) {
            showStatus('Aguarde, a aplicação ainda está carregando.', true);
            return;
        }

        const bytes = new Uint8Array(await file.arrayBuffer());
        const pointer = Module._malloc(bytes.length);
        if (pointer === 0) {
            showStatus('Memória insuficiente para abrir ' + file.name + '.', true);
            return;
        }

        // HEAPU8 é lido depois do malloc porque a memória pode ter crescido
        Module.HEAPU8.set(bytes, pointer);
        const loaded = Module._api_load_image(pointer, bytes.length);
        // O C já copiou os pixels decodificados, então o arquivo pode sair
        Module._free(pointer);

        if (loaded) {
            frame.classList.add('has-image');
            showStatus(file.name, false);
        } else {
            showStatus('Não foi possível abrir ' + file.name + '. Use uma imagem PNG, JPG ou BMP.', true);
        }
    }

    fileInput.addEventListener('change', function () {
        const file = fileInput.files[0];
        if (file) {
            loadFile(file);
        }
        // Permite escolher o mesmo arquivo de novo
        fileInput.value = '';
    });

    function hasFiles(event) {
        return event.dataTransfer && Array.from(event.dataTransfer.types).includes('Files');
    }

    function stopDragging() {
        dragDepth = 0;
        dropzone.classList.remove('is-dragging');
    }

    // Captura na janela para aceitar a foto solta em qualquer lugar da página
    // e antes do SDL, que também escuta soltar arquivos no canvas
    window.addEventListener('dragenter', function (event) {
        if (!hasFiles(event)) {
            return;
        }
        event.preventDefault();
        dragDepth++;
        dropzone.classList.add('is-dragging');
    }, true);

    window.addEventListener('dragover', function (event) {
        if (!hasFiles(event)) {
            return;
        }
        // Sem isso o navegador abre a foto numa aba nova
        event.preventDefault();
        event.dataTransfer.dropEffect = 'copy';
    }, true);

    window.addEventListener('dragleave', function (event) {
        if (!hasFiles(event)) {
            return;
        }
        dragDepth = Math.max(0, dragDepth - 1);
        if (dragDepth === 0) {
            stopDragging();
        }
    }, true);

    window.addEventListener('drop', function (event) {
        if (!hasFiles(event)) {
            return;
        }
        event.preventDefault();
        event.stopPropagation();
        stopDragging();
        const file = event.dataTransfer.files[0];
        if (file) {
            loadFile(file);
        }
    }, true);
})();
