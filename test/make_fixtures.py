import sys, numpy as np
from astropy.io import fits
fx = sys.argv[1]
h, w = 48, 64
img = (np.arange(h)[:, None] * 1000 + np.arange(w)[None, :] * 10).astype(np.uint16)   # rows differ, so a flip shows
img[2:8, 2:8] = 60000                                                                  # bright square near the top-left
def save(name, data, roworder):
    hdr = fits.Header(); hdr['ROWORDER'] = roworder; hdr['BAYERPAT'] = 'RGGB'
    fits.HDUList([fits.PrimaryHDU(), fits.CompImageHDU(data, header=hdr, compression_type='RICE_1')]).writeto(fx + '\\' + name, overwrite=True)
# PixInsight picks a reader from everything after the first dot, so every name that should reach
# the FZ module carries a ".t" before ".fits.fz". gradient_undotted.fits.fz shows the limitation:
# PixInsight hands it to its FITS module, which cannot read it.
save('gradient_top-down.t.fits.fz', img, 'TOP-DOWN')
save('gradient_bottom-up.t.fits.fz', img[::-1], 'BOTTOM-UP')   # same picture, rows stored the other way
save('gradient [brackets] (1).t.fits.fz', img, 'TOP-DOWN')
save('gradient_accent_é.t.fits.fz', img, 'TOP-DOWN')
save('gradient_undotted.fits.fz', img, 'TOP-DOWN')
fits.PrimaryHDU(img, header=fits.Header([('ROWORDER', 'TOP-DOWN')])).writeto(fx + '\\gradient_plain.fits', overwrite=True)
print('top row sig', int(sum(int(v) * (i + 1) for i, v in enumerate(img[0]))), 'sum', int(img.astype(np.uint64).sum()))
