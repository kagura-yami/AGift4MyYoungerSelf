const fs = require('node:fs');
const file = 'Frontend/implementations/typescript/src/player.ts';
const source = fs.readFileSync(file, 'utf8');
const original = 'new Config({ useUrlParams: true })';
if (!source.includes(original)) throw new Error('Pinned frontend config changed; review mouse defaults before building.');
fs.writeFileSync(file, source.replace(original,
    'new Config({ useUrlParams: true, initialSettings: { HoveringMouse: true } })'));
