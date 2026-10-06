"""Author a compact flat-color atlas for the imported Son mesh's existing UV swatches."""
from pathlib import Path
from PIL import Image, ImageDraw
root=Path(__file__).resolve().parents[2]
out=root/'game/Content/Characters/Materials'
out.mkdir(parents=True,exist_ok=True)
image=Image.new('RGB',(1024,1024),(218,181,139))
draw=ImageDraw.Draw(image)
# UV swatches measured from source FBX: skin, face details, hair, clothes and shoes.
palette=[(.594,.747,(237,181,139)),(.594,.705,(212,143,110)),
 (.631,.753,(78,49,36)),(.631,.729,(248,236,209)),
 (.260,.360,(84,53,38)),(.245,.864,(213,145,48)),
 (.602,.322,(49,72,91)),(.162,.063,(237,218,181)),
 (.237,.283,(75,53,40)),(.257,.110,(235,218,184)),
 (.199,.022,(38,32,29)),(.811,.561,(245,207,109)),(.729,.561,(238,184,62))]
for u,v,color in palette:
 x,y=round(u*1024),round((1-v)*1024)
 draw.rectangle((x-9,y-9,x+9,y+9),fill=color)
image.save(out/'T_Son_Palette.png')
print(out/'T_Son_Palette.png')
