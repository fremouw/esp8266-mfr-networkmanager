var upcomingVersion = '1.0.0';

var gulp = require('gulp'),
   uglify = require('gulp-uglify'),
   jshint = require('gulp-jshint'),
   gzip = require('gulp-gzip'),
   nodemon = require('gulp-nodemon'),
   del = require('del'),
   livereload = require('gulp-livereload'),
   htmlmin = require('gulp-htmlmin'),
   cleanCSS = require('gulp-clean-css');

gulp.task('clean', function() {
    del(['../data/fs']);
});

gulp.task('minify-css', function() {
  return gulp.src('fs/**/*.css')
    .pipe(cleanCSS({compatibility: 'ie8'}))
    .pipe(gzip())
    .pipe(gulp.dest('../data/fs'))
});

gulp.task('minify-html', function() {
  return gulp.src('fs/**/*.html')
    .pipe(htmlmin({collapseWhitespace: true}))
    //.pipe(cleanCSS({compatibility: 'ie8'}))
    .pipe(gzip())
    .pipe(gulp.dest('../data/fs'))
});

gulp.task('build', function () {
    gulp.src('fs/**/*.js')
    // .pipe(jshint())
    // .pipe(jshint.reporter('default'))
    .pipe(uglify())
    .pipe(gzip())
    .pipe(gulp.dest('../data/fs'));

});

// gulp.task('minify-css', function() {
//   return gulp.src('styles/*.css')
//     .pipe(cleanCSS({compatibility: 'ie8'}))
//     .pipe(gulp.dest('dist'));
// });

gulp.task('default', [ 'clean', 'minify-html', 'minify-css', 'build' ], function() {
});

gulp.task('watch', function(){
      gulp.watch('fs/**/*.js', ['build']);
});

gulp.task('nodemon', function () {
    nodemon({
        script: 'fs/',
        // watch: ['build/main.js'],
        verbose: true,
        ext: 'js',
        env: { 'NODE_ENV': 'development', 'NODE_CONFIG_DIR': 'app/config' },
    })
    .on('start', ['watch'])
    .on('change', ['watch'])
    .on('restart', function () {
        setTimeout(function () {
            livereload.changed();
           }, 1000);
        console.log('restarted!');
     });
});

// Default Task
// gulp.task('default', ['nodemon']);
gulp.task('serve', function() {
    nodemon({
      watch: ['fs/**/*.html'],
      env: { 'NODE_ENV': 'development' },
    })
    .on('restart', function () {
        console.log('restarted');
     });

});
