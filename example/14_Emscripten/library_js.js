mergeInto(LibraryManager.library, {
  
  uploadFlipped: function(img) {
    GLctx.pixelStorei(GLctx.UNPACK_FLIP_Y_WEBGL, true);
    GLctx.texImage2D(GLctx.TEXTURE_2D, 0, GLctx.RGBA, GLctx.RGBA, GLctx.UNSIGNED_BYTE, img);
    GLctx.pixelStorei(GLctx.UNPACK_FLIP_Y_WEBGL, false);
  },

  load_texture_from_url__deps: ['uploadFlipped'],
  load_texture_from_url: function(glTexture, url, outW, outH) {
    var img = new Image();
    img.onload = function() {
      HEAPU32[outW>>2] = img.width;
      HEAPU32[outH>>2] = img.height;
      GLctx.bindTexture(GLctx.TEXTURE_2D, GL.textures[glTexture]);
      _uploadFlipped(img);
    };
    img.src = UTF8ToString(url);
  }
});
