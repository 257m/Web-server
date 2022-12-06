const importObject = {
  imports: {
    imported_func(arg) {
      console.log(arg);
    },
  },
};

fetch("main.wasm")
  .then((response) => response.arrayBuffer())
  .then((bytes) => WebAssembly.instantiate(bytes, importObject))
  .then((result) => console.log(result.instance.exports.add(1, 2)));