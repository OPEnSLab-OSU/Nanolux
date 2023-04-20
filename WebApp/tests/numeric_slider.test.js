import { h } from 'preact';
import NumericSlider from '../src/components/numeric_slider';
import { shallow } from 'enzyme';


describe('Initial Test of the NumericSlider', () => {
    test('Slider renders 2 input items', () => {
        const context = shallow(<NumericSlider />);
        expect(context.find('slider')).toBeTruthy();
        expect(context.find('spinner')).toBeTruthy();
    });
});
