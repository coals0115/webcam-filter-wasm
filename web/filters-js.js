/**
 * JavaScript 필터 구현 (성능 비교용)
 * 순수 Canvas API와 JavaScript로 구현
 */

const FiltersJS = {
    /**
     * 흑백(Grayscale) 필터 - JavaScript 버전
     * ITU-R BT.709 표준 사용
     *
     * @param {ImageData} imageData - Canvas ImageData 객체
     */
    applyGrayscale(imageData) {
        const data = imageData.data;
        const length = data.length;

        for (let i = 0; i < length; i += 4) {
            const r = data[i];
            const g = data[i + 1];
            const b = data[i + 2];
            // Alpha는 i + 3

            // ITU-R BT.709 표준 공식 (WASM과 동일한 알고리즘)
            const gray = Math.floor(0.2126 * r + 0.7152 * g + 0.0722 * b);

            data[i] = gray;
            data[i + 1] = gray;
            data[i + 2] = gray;
            // Alpha는 그대로 유지
        }
    },

    /**
     * 좌우반전(Horizontal Flip) 필터 - JavaScript 버전
     *
     * @param {ImageData} imageData - Canvas ImageData 객체
     */
    applyHorizontalFlip(imageData) {
        const data = imageData.data;
        const width = imageData.width;
        const height = imageData.height;

        for (let y = 0; y < height; y++) {
            for (let x = 0; x < width / 2; x++) {
                const leftIdx = (y * width + x) * 4;
                const rightIdx = (y * width + (width - 1 - x)) * 4;

                // RGBA 각 채널을 교환
                for (let c = 0; c < 4; c++) {
                    const temp = data[leftIdx + c];
                    data[leftIdx + c] = data[rightIdx + c];
                    data[rightIdx + c] = temp;
                }
            }
        }
    }
};

// 전역 스코프에 노출
window.FiltersJS = FiltersJS;
