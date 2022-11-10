/** Large part of the code (the server part) is from:
 *  https://developer.mozilla.org/en-US/docs/Learn/Server-side/Node_server_without_framework
 * 
 */

const fs = require('fs');
const http = require('http');
const path = require('path');
const { exit } = require('process');
var exec = require('child_process').exec;

let args = process.argv.slice(2);

const waitFlag = args.includes('--wait')
const verboseFlag = args.includes('--verbose');

if(args.includes('--help') || args.includes('-h')){
  console.log("Usage: node test.js [--wait] [--verbose] [--help]");
  console.log("  --wait: Keep the server running after the tests are done. (must be killed manually)");
  console.log("  --verbose: Print verbose test results");
  console.log("  --help: Print this help text");
  exit(0);
}

/** START OF PART FROM MDN */
const PORT = 0;

const MIME_TYPES = {
  default: 'application/octet-stream',
  html: 'text/html; charset=UTF-8',
  js: 'application/javascript; charset=UTF-8',
  xml: 'application/xml; charset=UTF-8',
  css: 'text/css',
  png: 'image/png',
  jpg: 'image/jpg',
  gif: 'image/gif',
  ico: 'image/x-icon',
  svg: 'image/svg+xml',
};

const STATIC_PATH = path.join(process.cwd(), './tests');

const toBool = [() => true, () => false];

const prepareFile = async (url) => {
  const paths = [STATIC_PATH, url];
  if (url.endsWith('/')) paths.push('index.html');
  const filePath = path.join(...paths);
  const pathTraversal = !filePath.startsWith(STATIC_PATH);
  const exists = await fs.promises.access(filePath).then(...toBool);
  const found = !pathTraversal && exists;
  const streamPath = found ? filePath : STATIC_PATH + '/404.html';
  const ext = path.extname(streamPath).substring(1).toLowerCase();
  const stream = fs.createReadStream(streamPath);
  return { found, ext, stream };
};

var srv = http.createServer(async (req, res) => {
  const file = await prepareFile(req.url);
  const statusCode = file.found ? 200 : 404;
  const mimeType = MIME_TYPES[file.ext] || MIME_TYPES.default;
  res.writeHead(statusCode, { 'Content-Type': mimeType });
  file.stream.pipe(res);
  // console.log(`${req.method} ${req.url} ${statusCode}`);

  /** END OF PART FROM MDN */
}).listen(PORT, async () => { 
  console.log(`Server running at http://127.0.0.1:${srv.address().port}/`);
  let results = await performTests();

  printResults(results, verboseFlag);
  
  if(!waitFlag)
    srv.close();
});


async function performTests() {
  return new Promise(async (resolve, reject) => {
    const files = fs.readdirSync(STATIC_PATH);
    if (!files) {
      console.error("Could not list the directory.", err);
      reject(err);
    }

    testFiles = files.filter(file => file.endsWith('.json'));

    let results = [];

    for(const file of testFiles) {

      let fileNoExt = file.replace('.json', '');
      let testPath = path.join(STATIC_PATH, file);
      let testFile = fs.readFileSync(testPath, 'utf8');
      let test = JSON.parse(testFile);
      

      let feedOriginal = test.feedFile;
      let feedUrl = test.feedUrl;

      // Feed file stuff
      if(feedOriginal){
        let feedLines = feedOriginal.map( line => `http://localhost:${srv.address().port}/${line}`);
        let feed = feedLines.join('\n');
        fs.writeFileSync(path.join(STATIC_PATH,`${fileNoExt}.feed`), feed);
        
      }else if(feedUrl){
        test.args = test.args.replace('FEEDURL', `http://localhost:${srv.address().port}/${feedUrl}`);
      }else{
        console.log(`No feed file or url specified. (Skipping test ${fileNoExt}`);
        continue;
      }

      let executionResult = await runTest(test);
      let result = new TestResult();
      
      //determine if the test passed or failed
      if(test.returnCode == 0){
        let expectedPath = path.join(STATIC_PATH, `${fileNoExt}.out`);
        let expectedOutput;
        try{
          expectedOutput = fs.readFileSync(expectedPath, 'utf8');
        } catch (err) {
          console.error(`Could not read the expected output file (${fileNoExt}.out). (skipping test ${fileNoExt})`);
          continue;
        }
        result.passed = executionResult.returnCode == 0 && isOutputCorrect(executionResult.stdout, expectedOutput);
      }else{
        result.passed = executionResult.returnCode == test.returnCode;
      }
      
      result.testName = fileNoExt;
      result.executionResult = executionResult;
      result.expectedReturnCode = test.returnCode;

      results.push(result);
    }
    resolve(results);
  });
}

class TestResult {
  constructor() {
    this.testName = "";
    this.executionResult = null;
    this.passed = false;
    this.expectedReturnCode = 0;
  }
}

class ExecutionResult {
  constructor() {
    this.returnCode = 0;
    this.stdout = '';
    this.stderr = '';
  }
}

function isOutputCorrect(stdout, expected) {
  stdout = stdout.replace(/\s/g, "");
  expected = expected.replace(/\s/g, "");
  return stdout == expected;
}


async function runTest(test) {
  let args = test.args;
  return new Promise((resolve, reject) => {
    exec("./feedreader "+args, (error, stdout, stderr) => {
      let result = new ExecutionResult();
      
      result.returnCode = error ? error.code : 0;
      result.stdout = stdout;
      result.stderr = stderr;

      resolve(result);
    });
  });
}

function printResults(results, verbose) {
  console.log("======= Test results =======");
  for(const result of results){
    console.log(`Test ${result.testName}: ${result.passed ? "\x1b[32mOK" : "\x1b[1m\x1b[31mFAIL"}\x1b[0m`);
  }

  console.log("========= Summary =========");
  let passed = results.filter(result => result.passed);
  console.log(`Passed: ${passed.length}/${results.length}`);

  if(verbose)
    console.log("Verbose info:", results);
}